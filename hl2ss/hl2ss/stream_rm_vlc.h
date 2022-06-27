
#pragma once

#include "researchmode/ResearchModeApi.h"
#include <WinSock2.h>

#include <winrt/Windows.Perception.Spatial.h>

void RM_VLC_Stream_Mode0(IResearchModeSensor* sensor, SOCKET clientsocket);
void RM_VLC_Stream_Mode1(IResearchModeSensor* sensor, SOCKET clientsocket, winrt::Windows::Perception::Spatial::SpatialLocator const& locator, winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& world);
void RM_VLC_Stream_Mode2(IResearchModeSensor* sensor, SOCKET clientsocket);
