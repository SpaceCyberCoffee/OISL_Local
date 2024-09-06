#ifndef CFDP_H
#define CFDP_H

void sendFile(const char *fileContent, const size_t fileSize);

const int networkDelay = 100000; // 100 ms
double transferSpeedMbps = 100.0; // Transfer speed in Mbps

#endif
