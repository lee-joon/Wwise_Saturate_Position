/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2023 Audiokinetic Inc.
*******************************************************************************/

#include "pos_SatFX.h"
#include "../pos_SatConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* Createpos_SatFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, pos_SatFX());
}

AK::IAkPluginParam* Createpos_SatFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, pos_SatFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(pos_SatFX, AkPluginTypeEffect, pos_SatConfig::CompanyID, pos_SatConfig::PluginID)

pos_SatFX::pos_SatFX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
	, m_Distance(0)
{
}

pos_SatFX::~pos_SatFX()
{
}

AKRESULT pos_SatFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (pos_SatFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    return AK_Success;
}

AKRESULT pos_SatFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT pos_SatFX::Reset()
{
    return AK_Success;
}

AKRESULT pos_SatFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bCanProcessObjects = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void pos_SatFX::Execute(const AkAudioObjects& io_objects)
{
	//for (AkUInt32 objIdx = 0; objIdx < io_objects.uNumObjects; ++objIdx)
	//{
	//	const AkUInt32 uNumChannels = io_objects.ppObjectBuffers[objIdx]->NumChannels();

	//	AkUInt16 uFramesProcessed;
	//	for (AkUInt32 i = 0; i < uNumChannels; ++i)
	//	{
	//		// Peek into object metadata if desired.
	//		AkAudioObject* pObject = io_objects.ppObjects[objIdx];
	//		
	//		AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_objects.ppObjectBuffers[objIdx]->GetChannel(i);

	//		uFramesProcessed = 0;
	//		while (uFramesProcessed < io_objects.ppObjectBuffers[objIdx]->uValidFrames)
	//		{
	//			// Execute DSP in-place here
	//			++uFramesProcessed;
	//		}
	//	}
	//}
	AKASSERT(io_objects.uNumObjects < 1);

	for (AkUInt32 objIdx = 0; objIdx < io_objects.uNumObjects; ++objIdx)
	{
		AkAudioObject* temp_Audiobuffer = io_objects.ppObjects[objIdx];
		AkVector ObjVector = temp_Audiobuffer->positioning.threeD.xform.Position();
		//ObjVector = ObjVector * 0.01f;
		AkUInt32 Distance = (ObjVector.X * ObjVector.X) + (ObjVector.Y * ObjVector.Y) + (ObjVector.Z * ObjVector.Z);

		if (Distance < powf(m_pParams->NonRTPC.Distance, 2.0f))
		{
			m_satBuffer.push_back(SatuBuffer(io_objects.ppObjectBuffers[objIdx], Distance));
		}
	}

	std::vector<SatuBuffer>::iterator iter;

	for (iter = m_satBuffer.begin(); iter < m_satBuffer.end(); ++iter)
	{
		const AkUInt32 uNumChannels = iter->pInBuffer->NumChannels();
		AkUInt16 uFramesProcessed;

		for (int Chan = 0; Chan < uNumChannels; ++Chan)
		{
			AkReal32* AK_RESTRICT pBuffer = (AkReal32 * AK_RESTRICT)iter->pInBuffer->GetChannel(Chan);

			uFramesProcessed = 0;
			while (uFramesProcessed < iter->pInBuffer->uValidFrames)
			{

				pBuffer[uFramesProcessed] = Util.SaturateFunc(pBuffer[uFramesProcessed], 10.0f - (iter->Distance / powf(m_pParams->NonRTPC.Distance, 2.0f)) * 9.0f);
				++uFramesProcessed;
			}

		}
	}

	m_satBuffer.clear();
}
