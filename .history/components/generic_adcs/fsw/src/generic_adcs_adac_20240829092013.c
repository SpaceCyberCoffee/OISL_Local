/*******************************************************************************
** Purpose:
**   This file contains the source code for the attitude determination and
**   attitude control routines of the Generic ADCS application.
**
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include "generic_adcs_app.h"
#include "generic_adcs_utilities.h"
#include "generic_adcs_adac.h"

static void AD_imu(const Generic_ADCS_DI_Imu_Tlm_Payload_t *DI_IMU, Generic_ADCS_AD_Imu_Tlm_Payload_t *AD_IMU);
static void AD_mag(const Generic_ADCS_DI_Mag_Tlm_Payload_t *DI_Mag, Generic_ADCS_AD_Mag_Tlm_Payload_t *AD_Mag);
static void AD_sol(const Generic_ADCS_DI_Fss_Tlm_Payload_t *DI_FSS, const Generic_ADCS_DI_Css_Tlm_Payload_t *DI_CSS, 
    Generic_ADCS_AD_Sol_Tlm_Payload_t *AD_Sol);
static void AD_oisl(Generic_ADCS_DI_OISL_Tlm_Payload_t *DI_OISL, const Generic_ADCS_DI_St_Tlm_Payload_t *DI_St, Generic_ADCS_AD_ISL_Tlm_Payload_t *AD_ISL);
static void AD_to_GNC(const Generic_ADCS_AD_Tlm_Payload_t *AD, Generic_ADCS_GNC_Tlm_Payload_t *GNC);
static void AC_bdot(Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_Bdot_Tlm_t *AC_bdot);
static void AC_sunsafe(Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_Sunsafe_Tlm_t *ACS);
static void AC_oisl(Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_OISL_Tlm_t *AC_OISL, const char *F_or_B);
static void AC_h_mgmt(Generic_ADCS_GNC_Tlm_Payload_t *GNC);

void Generic_ADCS_init_attitude_determination_and_attitude_control(FILE *in, Generic_ADCS_AD_Tlm_Payload_t *AD, 
    Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_Tlm_Payload_t *ACS)
{
    char junk[512], newline;
    // AD
    fscanf(in, "%[^\n]%[\n]", junk, &newline);
    fscanf(in, "%lf%[^\n]%[\n]", &AD->Imu.alpha, junk, &newline);
    AD->Imu.init = 0;
    // GNC
    fscanf(in, "%[^\n]%[\n]", junk, &newline);
    fscanf(in, "%lf%[^\n]%[\n]", &GNC->DT, junk, &newline);
    fscanf(in, "%lf%[^\n]%[\n]", &GNC->MaxMcmd, junk, &newline);
    // AC Bdot
    fscanf(in, "%[^\n]%[\n]", junk, &newline);
    fscanf(in, "%lf %lf%[^\n]%[\n]", &ACS->Bdot.b_range, &ACS->Bdot.Kb, junk, &newline);
    // AC Sunsafe
    fscanf(in, "%[^\n]%[\n]", junk, &newline);
    fscanf(in, "%lf %lf %lf %lf %lf %lf%[^\n]%[\n]", &ACS->Sunsafe.Kp[0], &ACS->Sunsafe.Kp[1], &ACS->Sunsafe.Kp[2], 
        &ACS->Sunsafe.Kr[0], &ACS->Sunsafe.Kr[1], &ACS->Sunsafe.Kr[2], junk, &newline);
    fscanf(in, "%lf %lf %lf %lf %lf %lf %lf%[^\n]%[\n]", &ACS->Sunsafe.sside[0], &ACS->Sunsafe.sside[1], &ACS->Sunsafe.sside[2], &ACS->Sunsafe.vmax, 
        &ACS->Sunsafe.cmd_wbn[0], &ACS->Sunsafe.cmd_wbn[1], &ACS->Sunsafe.cmd_wbn[2], junk, &newline);
    for (int i = 0; i < 3; i++) {
        ACS->Sunsafe.therr[i] = ACS->Sunsafe.werr[i] = ACS->Sunsafe.Tcmd[i] = 0;
    }
    // AC OISL
    fscanf(in, "%[^\n]%[\n]", junk, &newline);
    fscanf(in, "%lf %lf %lf %lf %lf %lf%[^\n]%[\n]", &ACS->OISL.Kp[0], &ACS->OISL.Kp[1], &ACS->OISL.Kp[2], 
        &ACS->OISL.Kr[0], &ACS->OISL.Kr[1], &ACS->OISL.Kr[2], junk, &newline);
    fscanf(in, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf%[^\n]%[\n]", &ACS->OISL.sside_F[0], &ACS->OISL.sside_F[1], &ACS->OISL.sside_F[2], 
    &ACS->OISL.sside_B[0], &ACS->OISL.sside_B[1], &ACS->OISL.sside_B[2], &ACS->OISL.vmax, 
        &ACS->OISL.cmd_wbn[0], &ACS->OISL.cmd_wbn[1], &ACS->OISL.cmd_wbn[2], junk, &newline);
    for (int i = 0; i < 3; i++) {
        ACS->OISL.therr[i] = ACS->OISL.werr[i] = ACS->OISL.Tcmd[i] = 0;
    }
    // AC Momentum management
    fscanf(in, "%[^\n]%[\n]", junk, &newline);
    fscanf(in, "%lf %lf %lf %lf%[^\n]%[\n]", &GNC->Hmgmt.Kb, &GNC->Hmgmt.b_range, &GNC->Hmgmt.loFrac, &GNC->Hmgmt.hiFrac, junk, &newline);
}

void Generic_ADCS_execute_attitude_determination_and_attitude_control(const Generic_ADCS_DI_Tlm_Payload_t *DI, Generic_ADCS_AD_Tlm_Payload_t *AD, 
    Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_Tlm_Payload_t *ACS)
{
    AD_imu(&DI->Imu, &AD->Imu);
    AD_mag(&DI->Mag, &AD->Mag);
    AD_sol(&DI->Fss, &DI->Css, &AD->Sol);
    AD_oisl(&DI->OISL, &DI->St, &AD->Isl);

    AD_to_GNC(AD, GNC);
    for (int i = 0; i < 3; i++) GNC->HwhlB[i] = DI->Rw.HwhlB[i];
    for (int i = 0; i < 3; i++) GNC->HwhlMaxB[i] = DI->Rw.H_maxB[i];

    switch(GNC->Mode) {
    case BDOT_MODE:
        AC_bdot(GNC, &ACS->Bdot);
        break;
    
    case SUNSAFE_MODE:
        AC_sunsafe(GNC, &ACS->Sunsafe);
        break;

    case OISL_MODE_F:
        AC_oisl(GNC, &ACS->OISL, "FORWARD");
        break;

    case OISL_MODE_B:
        AC_oisl(GNC, &ACS->OISL, "BACKWARD");
        break;

    case PASSIVE_MODE:
    default:
        for (int i = 0; i < 3; i++) {
            GNC->Mcmd[i] = 0.0;
            GNC->Tcmd[i] = 0.0;
        }
        break;
    }
}

static void AD_oisl(Generic_ADCS_DI_OISL_Tlm_Payload_t *DI_OISL, const Generic_ADCS_DI_St_Tlm_Payload_t *DI_St, Generic_ADCS_AD_ISL_Tlm_Payload_t *AD_ISL)
{  
   QxV(DI_St->q, DI_OISL->ISL_vector_F, DI_OISL->ISL_vector_Body_F); // convert from sensor frame to body frame
   QxV(DI_St->q, DI_OISL->ISL_vector_B, DI_OISL->ISL_vector_Body_B); // convert from sensor frame to body frame
   for (int i = 0; i < 3; i++) AD_ISL->isl_F[i] = DI_OISL->ISL_vector_Body_F[i];
   for (int i = 0; i < 3; i++) AD_ISL->isl_B[i] = DI_OISL->ISL_vector_Body_B[i];
   // TEST
   double body_vector[3];
   for (int i = 0; i < 3; i++) body_vector[i] = DI_OISL->ISL_vector_Body_F[i] * 1758;
   double q_conj[4];
   for (int i = 0; i < 3; i++) q_conj[i] = -DI_St->q[i];
   q_conj[3] = DI_St->q[3];
   double eci_vector[3];
   QxV(q_conj, body_vector, eci_vector);
   OS_printf("Converted from ADCS FOrward position is: %f %f %f", ec)

}

static void AD_imu(const Generic_ADCS_DI_Imu_Tlm_Payload_t *DI_IMU, Generic_ADCS_AD_Imu_Tlm_Payload_t *AD_IMU)
{
    if (DI_IMU->valid) {
        AD_IMU->valid = 1;
        for (int i = 0; i < 3; i++) {
            AD_IMU->acc[i] = DI_IMU->acc[i];
        }

        if (AD_IMU->init == 0) {
            for (int i = 0; i < 3; i++) {
                AD_IMU->wbn[i] = DI_IMU->wbn[i];
            }
            AD_IMU->init = 1;
        } else {
            for (int i = 0; i < 3; i++) {
                AD_IMU->wbn[i] = AD_IMU->alpha * AD_IMU->wbn_prev[i] + (1 - AD_IMU->alpha) * DI_IMU->wbn[i];
            }
        }
        for (int i = 0; i < 3; i++) {
            AD_IMU->wbn_prev[i] = AD_IMU->wbn[i];
        }
    } else {
        AD_IMU->valid = 0;
    }
}

static void AD_mag(const Generic_ADCS_DI_Mag_Tlm_Payload_t *DI_Mag, Generic_ADCS_AD_Mag_Tlm_Payload_t *AD_Mag)
{
    /* AD very simple for magnetometer... there is only one mag and no fusion with anything else */
    for (int i = 0; i < 3; i++) {
        AD_Mag->bvb[i] = DI_Mag->bvb[i];
    }
}

static void AD_sol(const Generic_ADCS_DI_Fss_Tlm_Payload_t *DI_Fss, const Generic_ADCS_DI_Css_Tlm_Payload_t *DI_Css, Generic_ADCS_AD_Sol_Tlm_Payload_t *AD_Sol)
{
    if (DI_Fss->valid == 1) {
        AD_Sol->SunValid = 1;
        AD_Sol->FssValid = 1;
        AD_Sol->svb[0] = DI_Fss->svb[0];
        AD_Sol->svb[1] = DI_Fss->svb[1];
        AD_Sol->svb[2] = DI_Fss->svb[2];
    } else if (DI_Css->valid == 1) {
        AD_Sol->SunValid = 1;
        AD_Sol->FssValid = 0;
        AD_Sol->svb[0] = DI_Css->svb[0];
        AD_Sol->svb[1] = DI_Css->svb[1];
        AD_Sol->svb[2] = DI_Css->svb[2];
    } else {
        AD_Sol->SunValid = 0;
        AD_Sol->FssValid = 0;
        AD_Sol->svb[0] = 0.0;
        AD_Sol->svb[1] = 0.0;
        AD_Sol->svb[2] = 0.0;
    }
}

static void AD_to_GNC(const Generic_ADCS_AD_Tlm_Payload_t *AD, Generic_ADCS_GNC_Tlm_Payload_t *GNC)
{
    for (int i = 0; i < 3; i++) {
        GNC->bvb[i] = AD->Mag.bvb[i];
        GNC->svb[i] = AD->Sol.svb[i];
        GNC->wbn[i] = AD->Imu.wbn[i];
        GNC->ISL_vector_F[i] = AD->Isl.isl_F[i];
        GNC->ISL_vector_B[i] = AD->Isl.isl_B[i];
    }
    GNC->SunValid = AD->Sol.SunValid;
}

static void AC_bdot(Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_Bdot_Tlm_t *ACS)
{
    /* apply control only if b-field is in range */
    if (MAGV(GNC->bvb) > ACS->b_range) {
        for(int i = 0; i < 3; i++) {
            /* backward difference b-field derivative */
            ACS->bdot[i] = (GNC->bvb[i] - ACS->bold[i]) / GNC->DT;
            /* store old b-field */
            ACS->bold[i] = GNC->bvb[i];
            /* traditional b-dot algorithm */
            GNC->Mcmd[i] = -ACS->Kb * ACS->bdot[i] / MAGV(GNC->bvb);
            /* ensure wheels disabled */
            GNC->Tcmd[i] = 0.0;
        }
    } else {
        for (int i = 0; i < 3; i++) {
            GNC->Mcmd[i] = 0.0;
            GNC->Tcmd[i] = 0.0;
        }
    }
}

#define EPS 1.0E-6
static void AC_sunsafe(Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_Sunsafe_Tlm_t *ACS)
{
   int i;
   double u1[3] = {0.0, 0.0, 0.0}, err_b[3] = {0.0, 0.0, 0.0};      /* angle error calculation parameteres */
   double temp_sside[3] = {0.0, 0.0, 0.0};
   double SoS = 0.0;

/* .. Check that SS Vector is valid */
   if (GNC->SunValid) {

/* .. Form attitude error signals */
      SoS = VoV(GNC->svb, ACS->sside);
      if ((SoS > (EPS - 1.0)) && (SoS < (1.0 - EPS))) {
         VxV(GNC->svb, ACS->sside, ACS->therr);
      }
      else if (SoS >= (1.0 - EPS)) {
         ACS->therr[0] = 0.0;
         ACS->therr[1] = 0.0;
         ACS->therr[2] = 0.0;
         }
      else {
         err_b[0] = ACS->sside[1];
         err_b[1] = ACS->sside[2];
         err_b[2] = ACS->sside[0];
         if (fabs(err_b[0] - err_b[1]) < EPS && fabs(err_b[0] - err_b[2]) < EPS) {
            err_b[0] = -err_b[0];
         }
         VxV(ACS->sside, err_b, temp_sside);
         VxV(GNC->svb, temp_sside, ACS->therr);
      }
      
/* .. Closed-loop attitude control - PD Method */
      for(i = 0; i < 3; i++) {
         /* Clip attitude slew rates */
         u1[i] = Limit(ACS->Kp[i] / ACS->Kr[i] * ACS->therr[i], -ACS->vmax,ACS->vmax);
         ACS->werr[i] = GNC->wbn[i] - ACS->cmd_wbn[i];
         ACS->Tcmd[i] = -ACS->Kr[i] * (u1[i] + ACS->werr[i]);
      }

/* .. Apply Torque Command */
      for(i = 0; i < 3; i++) {
         GNC->Tcmd[i] = -ACS->Tcmd[i];
      }
   }

   else { /* during eclipse, reduce attitude rates only */

      for(i = 0; i < 3; i++) {
         ACS->werr[i] = GNC->wbn[i];
         ACS->Tcmd[i] = -ACS->Kr[i]* ACS->werr[i];
      }
      /* .. Apply Torque Command  */
      for (i = 0; i < 3; i++) {
         GNC->Tcmd[i] = -ACS->Tcmd[i];
      }
   }

   if (GNC->HmgmtOn) {
      AC_h_mgmt(GNC);
      for(i = 0; i < 3; i++) {
        GNC->Mcmd[i] = GNC->Hmgmt.Mcmd[i];
      }
   }
   else {
      for(i = 0; i < 3; i++) {
         GNC->Mcmd[i] = 0.0;
      }
   }

}


#define EPS_OISL 0.9998
// #define MIN_TORQUE_THRESHOLD 1.0E-5  // Define a small threshold to avoid negligible torque commands

static void AC_oisl(Generic_ADCS_GNC_Tlm_Payload_t *GNC, Generic_ADCS_AC_OISL_Tlm_t *ACS, const char *F_or_B)
{
   int i;
   double u1[3] = {0.0, 0.0, 0.0}, combined_err[3] = {0.0, 0.0, 0.0};      /* angle error calculation parameteres */
//    double temp_sside[3] = {0.0, 0.0, 0.0};
   double SoS = 0.0;
   double SoS_secondary = 0.0;
   double err_primary[3] = {0.0, 0.0, 0.0};
   double err_secondary[3] = {0.0, 0.0, 0.0};

    // Pointers to either forward or backward sside and ISL_vector based on F_or_B
    double *selected_sside;
    double *selected_ISL_vector;
    double *target_secondary;

    double SoS_F = VoV(GNC->ISL_vector_F, ACS->sside_F);
    double SoS_B = VoV(GNC->ISL_vector_B, ACS->sside_B);

    

    /* Select the appropriate sside and ISL_vector based on F_or_B */
    if (strcmp(F_or_B, "FORWARD") == 0) {
        selected_sside = ACS->sside_F;
        selected_ISL_vector = GNC->ISL_vector_F;
        SoS = SoS_F;
        target_secondary = ACS->sside_B;
        SoS_secondary = VoV(target_secondary, GNC->ISL_vector_F);
    } else {
        selected_sside = ACS->sside_B;
        selected_ISL_vector = GNC->ISL_vector_B;
        SoS = SoS_B;
        target_secondary = ACS->sside_F;
        SoS_secondary = VoV(target_secondary, GNC->ISL_vector_B);
    }

    /* Test writing to a file to be read by OISL HW model */
    char *filename_F = "/home/jstar/Desktop/github-nos3/sims/build/bin/VoV_FORWARD.txt";
    FILE *fp_F = fopen(filename_F, "w");
    fprintf(fp_F, "%f", SoS_F);
    fclose(fp_F);

    char *filename_B = "/home/jstar/Desktop/github-nos3/sims/build/bin/VoV_BACKWARD.txt";
    FILE *fp_B = fopen(filename_B, "w");
    fprintf(fp_B, "%f", SoS_B);
    fclose(fp_B);

    // printf("Scalar product between ISL V and desired b2 (should be close to 1): %f\n", SoS);
    

    // Calculate the primary alignment error
    if ((SoS > (EPS - 1.0)) && (SoS < (1.0 - EPS))) {
        VxV(selected_ISL_vector, selected_sside, err_primary);
    }

    // Calculate the secondary alignment error
    if (fabs(SoS_secondary) < EPS_OISL) {
        double temp_secondary[3];
        double neg_ISL_vector[3] = {-selected_ISL_vector[0], -selected_ISL_vector[1], -selected_ISL_vector[2]};
        VxV(target_secondary, neg_ISL_vector, temp_secondary);
        VxV(neg_ISL_vector, temp_secondary, err_secondary);
    }

    // Combine the errors
    for (i = 0; i < 3; i++) {
        combined_err[i] = err_primary[i] + err_secondary[i];
    }

    OS_printf("Secondary: %f\n", SoS_secondary);

    // If the alignment is within thresholds, zero the error
    if (SoS >= EPS_OISL && fabs(SoS_secondary) >= EPS_OISL) {
        for (i = 0; i < 3; i++) {
            ACS->therr[i] = 0.0;
        }
    } else {
        for (i = 0; i < 3; i++) {
            ACS->therr[i] = combined_err[i];
        }
    }

    /* .. Closed-loop attitude control - PD Method */
    for(i = 0; i < 3; i++) {
        /* Clip attitude slew rates */
        u1[i] = Limit(ACS->Kp[i] / ACS->Kr[i] * ACS->therr[i], -ACS->vmax, ACS->vmax);
        // if (fabs(u1[i]) < MIN_TORQUE_THRESHOLD) u1[i] = 0.0;  // Deadband for small torque commands
        ACS->werr[i] = GNC->wbn[i] - ACS->cmd_wbn[i];
        ACS->Tcmd[i] = -ACS->Kr[i] * (u1[i] + ACS->werr[i]);
    }

    /* .. Apply Torque Command */
    for(i = 0; i < 3; i++) {
        GNC->Tcmd[i] = -ACS->Tcmd[i];
    }

    if (GNC->HmgmtOn) {
        AC_h_mgmt(GNC);
        for(i = 0; i < 3; i++) {
            GNC->Mcmd[i] = GNC->Hmgmt.Mcmd[i];
        }
    }
    else {
        for(i = 0; i < 3; i++) {
            GNC->Mcmd[i] = 0.0;
        }
    }
}



static void AC_h_mgmt(Generic_ADCS_GNC_Tlm_Payload_t *GNC)
{

   double Herr[3] = {0.0, 0.0, 0.0};
   double bvb[3] = {0.0, 0.0, 0.0};
   double HxB[3] = {0.0, 0.0, 0.0};
   int i;

   if ( MAGV(GNC->bvb) > GNC->Hmgmt.b_range ) {
      /*Test if any axis needs to be momentum managed*/
      for(i=0;i<3;i++) {
         if (fabs(GNC->HwhlB[i]) > GNC->Hmgmt.hiFrac*fabs(GNC->HwhlMaxB[i])) {
            GNC->Hmgmt.mm_active[i] = 1;
         }
         if (fabs(GNC->HwhlB[i]) < GNC->Hmgmt.loFrac*fabs(GNC->HwhlMaxB[i])) {
            GNC->Hmgmt.mm_active[i] = 0;
         }
      }
      for(i = 0; i < 3; i++) {
         Herr[i] = 0.0;
         if (GNC->Hmgmt.mm_active[i] == 1) {
            Herr[i] = GNC->HwhlB[i];
         }
      }
      CopyUnitV(GNC->bvb, bvb);
      VxV(Herr,bvb,HxB);
      for(i = 0; i < 3; i++) {
         GNC->Hmgmt.Mcmd[i] = GNC->Hmgmt.Kb * HxB[i] / MAGV(GNC->bvb);
      }
   }
   else {
      for (i = 0; i < 3; i++) {
         GNC->Hmgmt.Mcmd[i] = 0.0;
      }
   }

}
