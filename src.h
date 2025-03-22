#include <windows.h>
#include <iostream>
#include <string>
#include <winuser.h>

void changeResolution(DWORD width, DWORD height, DWORD frequency) {
    DEVMODEA devMode;
    memset(&devMode, 0, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmDisplayFrequency = frequency;
    ChangeDisplaySettingsA(&devMode, 0);
}

void setHDR(UINT32 uid, bool enabled) {
    uint32_t pathCount, modeCount;

    uint8_t set[] = {0x0A, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x14, 0x81, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

    uint8_t request[] = {0x09, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x7C, 0x6F, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xDB, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00};

    if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount)) {
        DISPLAYCONFIG_PATH_INFO* pathsArray = nullptr;
        DISPLAYCONFIG_MODE_INFO* modesArray = nullptr;

        const size_t sizePathsArray = pathCount * sizeof(DISPLAYCONFIG_PATH_INFO);
        const size_t sizeModesArray = modeCount * sizeof(DISPLAYCONFIG_MODE_INFO);

        pathsArray = static_cast<DISPLAYCONFIG_PATH_INFO*>(std::malloc(sizePathsArray));
        modesArray = static_cast<DISPLAYCONFIG_MODE_INFO*>(std::malloc(sizeModesArray));

        if (pathsArray != nullptr && modesArray != nullptr) {
            std::memset(pathsArray, 0, sizePathsArray);
            std::memset(modesArray, 0, sizeModesArray);

            LONG queryRet = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, pathsArray, &modeCount, modesArray, 0);
            if (ERROR_SUCCESS == queryRet) {
                DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
                getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
                getColorInfo.header.size = sizeof(getColorInfo);

                DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
                setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
                setColorState.header.size = sizeof(setColorState);

                for (size_t i = 0; i < modeCount; i++) {
                    try {
                        if (modesArray[i].id != uid)
                            continue;
                        if (modesArray[i].infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET) {
                            DISPLAYCONFIG_MODE_INFO mode = modesArray[i];
                            getColorInfo.header.adapterId.HighPart = mode.adapterId.HighPart;
                            getColorInfo.header.adapterId.LowPart = mode.adapterId.LowPart;
                            getColorInfo.header.id = mode.id;

                            setColorState.header.adapterId.HighPart = mode.adapterId.HighPart;
                            setColorState.header.adapterId.LowPart = mode.adapterId.LowPart;
                            setColorState.header.id = mode.id;

                            if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&getColorInfo.header)) {
                                UINT32 value = enabled == true ? 1 : 0;
                                if (value != getColorInfo.advancedColorEnabled) {
                                    setColorState.enableAdvancedColor = enabled;
                                    DisplayConfigSetDeviceInfo(&setColorState.header);
                                    break;
                                }
                            }
                        }
                    } catch (const std::exception) {
                    }
                }
            }
            std::free(pathsArray);
            std::free(modesArray);
        }
        else {
            throw std::invalid_argument("No monitor found.");
        }
    }
}

void setHDR(bool enabled) {
    uint32_t pathCount, modeCount;

    uint8_t set[] = {0x0A, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x14, 0x81, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

    uint8_t request[] = {0x09, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x7C, 0x6F, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xDB, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00};

    if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount)) {
        DISPLAYCONFIG_PATH_INFO* pathsArray = nullptr;
        DISPLAYCONFIG_MODE_INFO* modesArray = nullptr;

        const size_t sizePathsArray = pathCount * sizeof(DISPLAYCONFIG_PATH_INFO);
        const size_t sizeModesArray = modeCount * sizeof(DISPLAYCONFIG_MODE_INFO);

        pathsArray = static_cast<DISPLAYCONFIG_PATH_INFO*>(std::malloc(sizePathsArray));
        modesArray = static_cast<DISPLAYCONFIG_MODE_INFO*>(std::malloc(sizeModesArray));

        if (pathsArray != nullptr && modesArray != nullptr) {
            std::memset(pathsArray, 0, sizePathsArray);
            std::memset(modesArray, 0, sizeModesArray);

            LONG queryRet = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, pathsArray, &modeCount, modesArray, 0);
            if (ERROR_SUCCESS == queryRet) {
                DISPLAYCONFIG_DEVICE_INFO_HEADER* setPacket = reinterpret_cast<DISPLAYCONFIG_DEVICE_INFO_HEADER*>(set);
                DISPLAYCONFIG_DEVICE_INFO_HEADER* requestPacket = reinterpret_cast<DISPLAYCONFIG_DEVICE_INFO_HEADER*>(request);
                for (size_t i = 0; i < modeCount; i++) {
                    try {
                        setHDR(modesArray[i].id, enabled);
                    } catch (const std::exception&) {
                    }
                }
            }
            std::free(pathsArray);
            std::free(modesArray);
        }
        else {
            throw std::invalid_argument("No monitor found.");
        }
    }
}
