#ifndef CFDP_H
#define CFDP_H

void sendFile(const char *fileContent, const size_t fileSize);

int networkDelay = 100000; // 100 ms
const double transferSpeedMbps = 100.0; // Transfer speed in Mbps

#endif
