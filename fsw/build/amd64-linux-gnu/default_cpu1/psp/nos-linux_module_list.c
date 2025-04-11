/* This file is generated via CMake - do not edit in place */
#include "cfe_psp_module.h"

extern CFE_PSP_ModuleApi_t CFE_PSP_eeprom_notimpl_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_port_notimpl_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_ram_notimpl_API;


CFE_StaticModuleLoadEntry_t CFE_PSP_BASE_MODULE_LIST[] =
{
{ .Name = "eeprom_notimpl", .Api = &CFE_PSP_eeprom_notimpl_API },
{ .Name = "port_notimpl", .Api = &CFE_PSP_port_notimpl_API },
{ .Name = "ram_notimpl", .Api = &CFE_PSP_ram_notimpl_API },

{ NULL }
};

/* END OF FILE */
