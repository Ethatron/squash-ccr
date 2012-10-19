/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is squash.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011-2012 The Initial Developer.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "format.h"

namespace squash {

  /* type-casting to native types */
#ifndef DX11
  TEXFORMAT::operator D3DFORMAT() const {
    switch (TEXFMT) {
      case TEXFMT_UNKNOWN              : return D3DFMT_UNKNOWN              ;

      case TEXFMT_R8G8B8               : return D3DFMT_R8G8B8               ;
      case TEXFMT_A8R8G8B8             : return D3DFMT_A8R8G8B8             ;
      case TEXFMT_A8R8G8B8srgb         : return D3DFMT_A8R8G8B8             ;
      case TEXFMT_X8R8G8B8             : return D3DFMT_X8R8G8B8             ;
      case TEXFMT_X8R8G8B8srgb         : return D3DFMT_X8R8G8B8             ;
      case TEXFMT_R5G6B5               : return D3DFMT_R5G6B5               ;
      case TEXFMT_X1R5G5B5             : return D3DFMT_X1R5G5B5             ;
      case TEXFMT_A1R5G5B5             : return D3DFMT_A1R5G5B5             ;
      case TEXFMT_A4R4G4B4             : return D3DFMT_A4R4G4B4             ;
      case TEXFMT_R3G3B2               : return D3DFMT_R3G3B2               ;
      case TEXFMT_A8                   : return D3DFMT_A8                   ;
      case TEXFMT_A8R3G3B2             : return D3DFMT_A8R3G3B2             ;
      case TEXFMT_X4R4G4B4             : return D3DFMT_X4R4G4B4             ;
      case TEXFMT_A2B10G10R10          : return D3DFMT_A2B10G10R10          ;
      case TEXFMT_A8B8G8R8             : return D3DFMT_A8B8G8R8             ;
      case TEXFMT_A8B8G8R8srgb         : return D3DFMT_A8B8G8R8             ;
      case TEXFMT_X8B8G8R8             : return D3DFMT_X8B8G8R8             ;
      case TEXFMT_X8B8G8R8srgb         : return D3DFMT_X8B8G8R8             ;
      case TEXFMT_G16R16               : return D3DFMT_G16R16               ;
      case TEXFMT_A2R10G10B10          : return D3DFMT_A2R10G10B10          ;
      case TEXFMT_A2B10G10R10_XR_BIAS  : return D3DFMT_A2B10G10R10_XR_BIAS  ;
      case TEXFMT_A16B16G16R16         : return D3DFMT_A16B16G16R16         ;
      case TEXFMT_A8P8                 : return D3DFMT_A8P8                 ;
      case TEXFMT_P8                   : return D3DFMT_P8                   ;
      case TEXFMT_L8                   : return D3DFMT_L8                   ;
      case TEXFMT_A8L8                 : return D3DFMT_A8L8                 ;
      case TEXFMT_A4L4                 : return D3DFMT_A4L4                 ;
      case TEXFMT_V8U8                 : return D3DFMT_V8U8                 ;
      case TEXFMT_L6V5U5               : return D3DFMT_L6V5U5               ;
      case TEXFMT_X8L8V8U8             : return D3DFMT_X8L8V8U8             ;
      case TEXFMT_Q8W8V8U8             : return D3DFMT_Q8W8V8U8             ;
      case TEXFMT_V16U16               : return D3DFMT_V16U16               ;
      case TEXFMT_A2W10V10U10          : return D3DFMT_A2W10V10U10          ;
      case TEXFMT_UYVY                 : return D3DFMT_UYVY                 ;
      case TEXFMT_YUY2                 : return D3DFMT_YUY2                 ;
      case TEXFMT_R8G8_B8G8            : return D3DFMT_R8G8_B8G8            ;
      case TEXFMT_G8R8_G8B8            : return D3DFMT_G8R8_G8B8            ;
      case TEXFMT_DXT1                 : return D3DFMT_DXT1                 ;
      case TEXFMT_DXT2                 : return D3DFMT_DXT2                 ;
      case TEXFMT_DXT3                 : return D3DFMT_DXT3                 ;
      case TEXFMT_DXT4                 : return D3DFMT_DXT4                 ;
      case TEXFMT_DXT5                 : return D3DFMT_DXT5                 ;
      case TEXFMT_ATI1                 : return D3DFMT_ATI1                 ;
      case TEXFMT_ATI2                 : return D3DFMT_ATI2                 ;
      case TEXFMT_D16_LOCKABLE         : return D3DFMT_D16_LOCKABLE         ;
      case TEXFMT_D32                  : return D3DFMT_D32                  ;
      case TEXFMT_D15S1                : return D3DFMT_D15S1                ;
      case TEXFMT_D24S8                : return D3DFMT_D24S8                ;
      case TEXFMT_D24X8                : return D3DFMT_D24X8                ;
      case TEXFMT_D24X4S4              : return D3DFMT_D24X4S4              ;
      case TEXFMT_D16                  : return D3DFMT_D16                  ;
      case TEXFMT_D32F_LOCKABLE        : return D3DFMT_D32F_LOCKABLE        ;
      case TEXFMT_D24FS8               : return D3DFMT_D24FS8               ;
      case TEXFMT_INTZ                 : return D3DFMT_INTZ                 ;
      case TEXFMT_DF24                 : return D3DFMT_DF24                 ;
      case TEXFMT_DF16                 : return D3DFMT_DF16                 ;
      case TEXFMT_RAWZ                 : return D3DFMT_RAWZ                 ;
      case TEXFMT_D32_LOCKABLE         : return D3DFMT_D32_LOCKABLE         ;
      case TEXFMT_S8_LOCKABLE          : return D3DFMT_S8_LOCKABLE          ;
      case TEXFMT_L16                  : return D3DFMT_L16                  ;
      case TEXFMT_VERTEXDATA           : return D3DFMT_VERTEXDATA           ;
      case TEXFMT_INDEX16              : return D3DFMT_INDEX16              ;
      case TEXFMT_INDEX32              : return D3DFMT_INDEX32              ;
      case TEXFMT_Q16W16V16U16         : return D3DFMT_Q16W16V16U16         ;
      case TEXFMT_MULTI2_ARGB8         : return D3DFMT_MULTI2_ARGB8         ;
      case TEXFMT_R16F                 : return D3DFMT_R16F                 ;
      case TEXFMT_G16R16F              : return D3DFMT_G16R16F              ;
      case TEXFMT_A16B16G16R16F        : return D3DFMT_A16B16G16R16F        ;
      case TEXFMT_R32F                 : return D3DFMT_R32F                 ;
      case TEXFMT_G32R32F              : return D3DFMT_G32R32F              ;
      case TEXFMT_A32B32G32R32F        : return D3DFMT_A32B32G32R32F        ;
      case TEXFMT_CxV8U8               : return D3DFMT_CxV8U8               ;
      case TEXFMT_A1                   : return D3DFMT_A1                   ;
      case TEXFMT_BINARYBUFFER         : return D3DFMT_BINARYBUFFER         ;
    }

    return D3DFMT_UNKNOWN;
  }
#else
  TEXFORMAT::operator DXGI_FORMAT() const {
    switch (TEXFMT) {
      case TEXFMT_UNKNOWN              : return DXGI_FORMAT_UNKNOWN;

      case TEXFMT_R1                   : return DXGI_FORMAT_R1_UNORM;
      case TEXFMT_R8                   : return DXGI_FORMAT_R8_UNORM;
      case TEXFMT_G8R8                 : return DXGI_FORMAT_R8G8_UNORM;
      case TEXFMT_R8G8B8               : break;
      case TEXFMT_A8R8G8B8             : return DXGI_FORMAT_B8G8R8A8_UNORM;
      case TEXFMT_X8R8G8B8             : return DXGI_FORMAT_B8G8R8X8_UNORM;
      case TEXFMT_R5G6B5               : return DXGI_FORMAT_B5G6R5_UNORM;
      case TEXFMT_X1R5G5B5             : break;
      case TEXFMT_A1R5G5B5             : return DXGI_FORMAT_B5G5R5A1_UNORM;
#if	defined(W8SUPPORT)
      case TEXFMT_A4R4G4B4             : return DXGI_FORMAT_B4G4R4A4_UNORM;
      case TEXFMT_X4R4G4B4             : return DXGI_FORMAT_B4G4R4A4_UNORM;
#elif	!defined(DX11_NORECAST)
      case TEXFMT_A4R4G4B4             : return DXGI_FORMAT_R16_TYPELESS;
      case TEXFMT_X4R4G4B4             : return DXGI_FORMAT_R16_TYPELESS;
#else
      case TEXFMT_A4R4G4B4             : break;
      case TEXFMT_X4R4G4B4             : break;
#endif
      case TEXFMT_R3G3B2               : break;
      case TEXFMT_A8R3G3B2             : break;
      case TEXFMT_A8B8G8R8             : return DXGI_FORMAT_R8G8B8A8_UNORM;
      case TEXFMT_X8B8G8R8             : break;
	
      case TEXFMT_A8R8G8B8srgb         : return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      case TEXFMT_X8R8G8B8srgb         : return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
      case TEXFMT_A8B8G8R8srgb         : return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      case TEXFMT_X8B8G8R8srgb         : break;

      case TEXFMT_A2B10G10R10          : break;
      case TEXFMT_A2R10G10B10          : return DXGI_FORMAT_R10G10B10A2_UNORM;
      case TEXFMT_A2B10G10R10_XR_BIAS  : return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
      case TEXFMT_R16                  : return DXGI_FORMAT_R16_UNORM;
      case TEXFMT_G16R16               : return DXGI_FORMAT_R16G16_UNORM;
      case TEXFMT_A16B16G16R16         : return DXGI_FORMAT_R16G16B16A16_UNORM;
      case TEXFMT_R32                  : return DXGI_FORMAT_R32_UINT;
      case TEXFMT_G32R32               : return DXGI_FORMAT_R32G32_UINT;
      case TEXFMT_B32G32R32            : return DXGI_FORMAT_R32G32B32_UINT;
      case TEXFMT_A32B32G32R32         : return DXGI_FORMAT_R32G32B32A32_UINT;

#if	defined(DX11_NORECAST)
      case TEXFMT_A1                   : break;
      case TEXFMT_A8                   : return DXGI_FORMAT_A8_UNORM;
#else
      case TEXFMT_A1                   : return DXGI_FORMAT_R1_UNORM;
      case TEXFMT_A8                   : return DXGI_FORMAT_A8_UNORM;
#endif

#if	defined(W8SUPPORT)
      case TEXFMT_P8                   : return DXGI_FORMAT_P8_UNORM;
      case TEXFMT_A8P8                 : return DXGI_FORMAT_A8P8_UNORM;
#elif	!defined(DX11_NORECAST)
      case TEXFMT_P8                   : return DXGI_FORMAT_R8_UNORM;
      case TEXFMT_A8P8                 : return DXGI_FORMAT_R8G8_UNORM;
#else
      case TEXFMT_P8                   : break;
      case TEXFMT_A8P8                 : break;
#endif

#if	defined(DX11_NORECAST)
      case TEXFMT_L8                   : break;
      case TEXFMT_L16                  : break;
      case TEXFMT_A4L4                 : break;
      case TEXFMT_A8L8                 : break;
#else
      case TEXFMT_L8                   : return DXGI_FORMAT_R8_UNORM;
      case TEXFMT_L16                  : return DXGI_FORMAT_R16_UNORM;
      case TEXFMT_A4L4                 : return DXGI_FORMAT_R8_TYPELESS;
      case TEXFMT_A8L8                 : return DXGI_FORMAT_R8G8_UNORM;
#endif

      case TEXFMT_R9G9B9E5             : return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

      case TEXFMT_CxV8U8               : break;
      case TEXFMT_L6V5U5               : break;
      case TEXFMT_X8L8V8U8             : break;
      case TEXFMT_A2W10V10U10          : break;
      case TEXFMT_U8                   : return DXGI_FORMAT_R8_SNORM;
      case TEXFMT_V8U8                 : return DXGI_FORMAT_R8G8_SNORM;
      case TEXFMT_Q8W8V8U8             : return DXGI_FORMAT_R8G8B8A8_SNORM;
      case TEXFMT_U16                  : return DXGI_FORMAT_R16_SNORM;
      case TEXFMT_V16U16               : return DXGI_FORMAT_R16G16_SNORM;
      case TEXFMT_Q16W16V16U16         : return DXGI_FORMAT_R16G16B16A16_SNORM;

      case TEXFMT_R11G11B10F           : return DXGI_FORMAT_R11G11B10_FLOAT;
      case TEXFMT_R16F                 : return DXGI_FORMAT_R16_FLOAT;
      case TEXFMT_G16R16F              : return DXGI_FORMAT_R16G16_FLOAT;
      case TEXFMT_A16B16G16R16F        : return DXGI_FORMAT_R16G16B16A16_FLOAT;
      case TEXFMT_R32F                 : return DXGI_FORMAT_R32_FLOAT;
      case TEXFMT_G32R32F              : return DXGI_FORMAT_R32G32_FLOAT;
      case TEXFMT_B32G32R32F           : return DXGI_FORMAT_R32G32B32_FLOAT;
      case TEXFMT_A32B32G32R32F        : return DXGI_FORMAT_R32G32B32A32_FLOAT;

      case TEXFMT_R32G8X24             : return DXGI_FORMAT_R32G8X24_TYPELESS;
      case TEXFMT_R32X8X24             : return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
      case TEXFMT_X32G8X24             : return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
      case TEXFMT_R24G8                : return DXGI_FORMAT_R24G8_TYPELESS;
      case TEXFMT_R24X8                : return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      case TEXFMT_X24G8                : return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

      case TEXFMT_UYVY                 : break;
      case TEXFMT_YUY2                 : break;
      case TEXFMT_R8G8_B8G8            : return DXGI_FORMAT_R8G8_B8G8_UNORM;
      case TEXFMT_G8R8_G8B8            : return DXGI_FORMAT_G8R8_G8B8_UNORM;

      case TEXFMT_DXT1                 : return DXGI_FORMAT_BC1_UNORM;
      case TEXFMT_DXT2                 : break;
      case TEXFMT_DXT3                 : return DXGI_FORMAT_BC2_UNORM;
      case TEXFMT_DXT4                 : break;
      case TEXFMT_DXT5                 : return DXGI_FORMAT_BC3_UNORM;
      case TEXFMT_DXTM                 : return DXGI_FORMAT_BC7_UNORM;
      case TEXFMT_ATI1                 : return DXGI_FORMAT_BC4_UNORM;
      case TEXFMT_ATI2                 : return DXGI_FORMAT_BC5_UNORM;
      case TEXFMT_DXTH                 : return DXGI_FORMAT_BC6H_UF16;

      case TEXFMT_DXT1srgb             : return DXGI_FORMAT_BC1_UNORM_SRGB;
      case TEXFMT_DXT2srgb             : break;
      case TEXFMT_DXT3srgb             : return DXGI_FORMAT_BC2_UNORM_SRGB;
      case TEXFMT_DXT4srgb             : break;
      case TEXFMT_DXT5srgb             : return DXGI_FORMAT_BC3_UNORM_SRGB;
      case TEXFMT_DXTMsrgb             : return DXGI_FORMAT_BC7_UNORM_SRGB;
      case TEXFMT_ATI1s                : return DXGI_FORMAT_BC4_SNORM;
      case TEXFMT_ATI2s                : return DXGI_FORMAT_BC5_SNORM;
      case TEXFMT_DXTHs                : return DXGI_FORMAT_BC6H_SF16;

      case TEXFMT_S8_LOCKABLE          : break;
      case TEXFMT_D15S1                : break;
      case TEXFMT_D16                  : return DXGI_FORMAT_D16_UNORM;
      case TEXFMT_D16_LOCKABLE         : break;
      case TEXFMT_D24S8                : return DXGI_FORMAT_D24_UNORM_S8_UINT;
      case TEXFMT_D24X8                : break;
      case TEXFMT_D24X4S4              : break;
      case TEXFMT_D24FS8               : break;
      case TEXFMT_D32                  : break;
      case TEXFMT_D32_LOCKABLE         : break;
      case TEXFMT_D32S8                : return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
      case TEXFMT_D32F_LOCKABLE        : return DXGI_FORMAT_D32_FLOAT;
      case TEXFMT_INTZ                 : break;
      case TEXFMT_DF24                 : break;
      case TEXFMT_DF16                 : break;
      case TEXFMT_RAWZ                 : break;

      case TEXFMT_VERTEXDATA           : break;
      case TEXFMT_INDEX16              : break;
      case TEXFMT_INDEX32              : break;
      case TEXFMT_MULTI2_ARGB8         : break;

      case TEXFMT_BINARYBUFFER         : break;
    }

    return DXGI_FORMAT_UNKNOWN;
  }
#endif

  /* assignment from native types */
#ifndef DX11
  TEXFORMAT &TEXFORMAT::operator = (const D3DFORMAT &dx9f) {
    TEXFMT = TEXFMT_UNKNOWN;

    switch (dx9f) {
      case D3DFMT_UNKNOWN              : TEXFMT = TEXFMT_UNKNOWN              ; break;
      case D3DFMT_R8G8B8               : TEXFMT = TEXFMT_R8G8B8               ; break;
      case D3DFMT_A8R8G8B8             : TEXFMT = TEXFMT_A8R8G8B8             ; break;
      case D3DFMT_X8R8G8B8             : TEXFMT = TEXFMT_X8R8G8B8             ; break;
      case D3DFMT_R5G6B5               : TEXFMT = TEXFMT_R5G6B5               ; break;
      case D3DFMT_X1R5G5B5             : TEXFMT = TEXFMT_X1R5G5B5             ; break;
      case D3DFMT_A1R5G5B5             : TEXFMT = TEXFMT_A1R5G5B5             ; break;
      case D3DFMT_A4R4G4B4             : TEXFMT = TEXFMT_A4R4G4B4             ; break;
      case D3DFMT_R3G3B2               : TEXFMT = TEXFMT_R3G3B2               ; break;
      case D3DFMT_A8                   : TEXFMT = TEXFMT_A8                   ; break;
      case D3DFMT_A8R3G3B2             : TEXFMT = TEXFMT_A8R3G3B2             ; break;
      case D3DFMT_X4R4G4B4             : TEXFMT = TEXFMT_X4R4G4B4             ; break;
      case D3DFMT_A2B10G10R10          : TEXFMT = TEXFMT_A2B10G10R10          ; break;
      case D3DFMT_A8B8G8R8             : TEXFMT = TEXFMT_A8B8G8R8             ; break;
      case D3DFMT_X8B8G8R8             : TEXFMT = TEXFMT_X8B8G8R8             ; break;
      case D3DFMT_G16R16               : TEXFMT = TEXFMT_G16R16               ; break;
      case D3DFMT_A2R10G10B10          : TEXFMT = TEXFMT_A2R10G10B10          ; break;
      case D3DFMT_A2B10G10R10_XR_BIAS  : TEXFMT = TEXFMT_A2B10G10R10_XR_BIAS  ; break;
      case D3DFMT_A16B16G16R16         : TEXFMT = TEXFMT_A16B16G16R16         ; break;
      case D3DFMT_A8P8                 : TEXFMT = TEXFMT_A8P8                 ; break;
      case D3DFMT_P8                   : TEXFMT = TEXFMT_P8                   ; break;
      case D3DFMT_L8                   : TEXFMT = TEXFMT_L8                   ; break;
      case D3DFMT_A8L8                 : TEXFMT = TEXFMT_A8L8                 ; break;
      case D3DFMT_A4L4                 : TEXFMT = TEXFMT_A4L4                 ; break;
      case D3DFMT_V8U8                 : TEXFMT = TEXFMT_V8U8                 ; break;
      case D3DFMT_L6V5U5               : TEXFMT = TEXFMT_L6V5U5               ; break;
      case D3DFMT_X8L8V8U8             : TEXFMT = TEXFMT_X8L8V8U8             ; break;
      case D3DFMT_Q8W8V8U8             : TEXFMT = TEXFMT_Q8W8V8U8             ; break;
      case D3DFMT_V16U16               : TEXFMT = TEXFMT_V16U16               ; break;
      case D3DFMT_A2W10V10U10          : TEXFMT = TEXFMT_A2W10V10U10          ; break;
      case D3DFMT_UYVY                 : TEXFMT = TEXFMT_UYVY                 ; break;
      case D3DFMT_YUY2                 : TEXFMT = TEXFMT_YUY2                 ; break;
      case D3DFMT_R8G8_B8G8            : TEXFMT = TEXFMT_R8G8_B8G8            ; break;
      case D3DFMT_G8R8_G8B8            : TEXFMT = TEXFMT_G8R8_G8B8            ; break;
      case D3DFMT_DXT1                 : TEXFMT = TEXFMT_DXT1                 ; break;
      case D3DFMT_DXT2                 : TEXFMT = TEXFMT_DXT2                 ; break;
      case D3DFMT_DXT3                 : TEXFMT = TEXFMT_DXT3                 ; break;
      case D3DFMT_DXT4                 : TEXFMT = TEXFMT_DXT4                 ; break;
      case D3DFMT_DXT5                 : TEXFMT = TEXFMT_DXT5                 ; break;
      case D3DFMT_ATI1                 : TEXFMT = TEXFMT_ATI1                 ; break;
      case D3DFMT_ATI2                 : TEXFMT = TEXFMT_ATI2                 ; break;
      case D3DFMT_D16_LOCKABLE         : TEXFMT = TEXFMT_D16_LOCKABLE         ; break;
      case D3DFMT_D32                  : TEXFMT = TEXFMT_D32                  ; break;
      case D3DFMT_D15S1                : TEXFMT = TEXFMT_D15S1                ; break;
      case D3DFMT_D24S8                : TEXFMT = TEXFMT_D24S8                ; break;
      case D3DFMT_D24X8                : TEXFMT = TEXFMT_D24X8                ; break;
      case D3DFMT_D24X4S4              : TEXFMT = TEXFMT_D24X4S4              ; break;
      case D3DFMT_D16                  : TEXFMT = TEXFMT_D16                  ; break;
      case D3DFMT_D32F_LOCKABLE        : TEXFMT = TEXFMT_D32F_LOCKABLE        ; break;
      case D3DFMT_D24FS8               : TEXFMT = TEXFMT_D24FS8               ; break;
      case D3DFMT_INTZ                 : TEXFMT = TEXFMT_INTZ                 ; break;
      case D3DFMT_DF24                 : TEXFMT = TEXFMT_DF24                 ; break;
      case D3DFMT_DF16                 : TEXFMT = TEXFMT_DF16                 ; break;
      case D3DFMT_RAWZ                 : TEXFMT = TEXFMT_RAWZ                 ; break;
      case D3DFMT_D32_LOCKABLE         : TEXFMT = TEXFMT_D32_LOCKABLE         ; break;
      case D3DFMT_S8_LOCKABLE          : TEXFMT = TEXFMT_S8_LOCKABLE          ; break;
      case D3DFMT_L16                  : TEXFMT = TEXFMT_L16                  ; break;
      case D3DFMT_VERTEXDATA           : TEXFMT = TEXFMT_VERTEXDATA           ; break;
      case D3DFMT_INDEX16              : TEXFMT = TEXFMT_INDEX16              ; break;
      case D3DFMT_INDEX32              : TEXFMT = TEXFMT_INDEX32              ; break;
      case D3DFMT_Q16W16V16U16         : TEXFMT = TEXFMT_Q16W16V16U16         ; break;
      case D3DFMT_MULTI2_ARGB8         : TEXFMT = TEXFMT_MULTI2_ARGB8         ; break;
      case D3DFMT_R16F                 : TEXFMT = TEXFMT_R16F                 ; break;
      case D3DFMT_G16R16F              : TEXFMT = TEXFMT_G16R16F              ; break;
      case D3DFMT_A16B16G16R16F        : TEXFMT = TEXFMT_A16B16G16R16F        ; break;
      case D3DFMT_R32F                 : TEXFMT = TEXFMT_R32F                 ; break;
      case D3DFMT_G32R32F              : TEXFMT = TEXFMT_G32R32F              ; break;
      case D3DFMT_A32B32G32R32F        : TEXFMT = TEXFMT_A32B32G32R32F        ; break;
      case D3DFMT_CxV8U8               : TEXFMT = TEXFMT_CxV8U8               ; break;
      case D3DFMT_A1                   : TEXFMT = TEXFMT_A1                   ; break;
      case D3DFMT_BINARYBUFFER         : TEXFMT = TEXFMT_BINARYBUFFER         ; break;
    }

    return *this;
  }
#else
  TEXFORMAT &TEXFORMAT::operator = (const DXGI_FORMAT &dx11f) {
    TEXFMT = TEXFMT_UNKNOWN;

    switch (dx11f) {
      case DXGI_FORMAT_UNKNOWN                    : TEXFMT = TEXFMT_UNKNOWN              ; break;

      case DXGI_FORMAT_R1_UNORM                   : TEXFMT = TEXFMT_R1                   ; break;
      case DXGI_FORMAT_R8_UNORM                   : TEXFMT = TEXFMT_R8                   ; break;
      case DXGI_FORMAT_R8G8_UNORM                 : TEXFMT = TEXFMT_G8R8                 ; break;

      case DXGI_FORMAT_R8G8B8A8_UNORM             : TEXFMT = TEXFMT_A8B8G8R8             ; break;
      case DXGI_FORMAT_B8G8R8A8_TYPELESS          : TEXFMT = TEXFMT_A8R8G8B8             ; break;
      case DXGI_FORMAT_B8G8R8A8_UNORM             : TEXFMT = TEXFMT_A8R8G8B8             ; break;
      case DXGI_FORMAT_B8G8R8X8_TYPELESS          : TEXFMT = TEXFMT_X8R8G8B8             ; break;
      case DXGI_FORMAT_B8G8R8X8_UNORM             : TEXFMT = TEXFMT_X8R8G8B8             ; break;

      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB        : TEXFMT = TEXFMT_A8B8G8R8srgb         ; break;
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB        : TEXFMT = TEXFMT_A8R8G8B8srgb         ; break;
      case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB        : TEXFMT = TEXFMT_X8R8G8B8srgb         ; break;

      case DXGI_FORMAT_B5G6R5_UNORM               : TEXFMT = TEXFMT_R5G6B5               ; break;
      case DXGI_FORMAT_B5G5R5A1_UNORM             : TEXFMT = TEXFMT_A1R5G5B5             ; break;
#ifdef	W8SUPPORT
      case DXGI_FORMAT_B4G4R4A4_UNORM             : TEXFMT = TEXFMT_A4R4G4B4             ; break;
#endif
      case DXGI_FORMAT_R10G10B10A2_UNORM          : TEXFMT = TEXFMT_A2R10G10B10          ; break;
      case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM : TEXFMT = TEXFMT_A2B10G10R10_XR_BIAS  ; break;
      case DXGI_FORMAT_R16_UNORM                  : TEXFMT = TEXFMT_R16                  ; break;
      case DXGI_FORMAT_R16G16_UNORM               : TEXFMT = TEXFMT_G16R16               ; break;
      case DXGI_FORMAT_R16G16B16A16_UNORM         : TEXFMT = TEXFMT_A16B16G16R16         ; break;
      case DXGI_FORMAT_R32_UINT                   : TEXFMT = TEXFMT_R32                  ; break;
      case DXGI_FORMAT_R32G32_UINT                : TEXFMT = TEXFMT_G32R32               ; break;
      case DXGI_FORMAT_R32G32B32_UINT             : TEXFMT = TEXFMT_B32G32R32            ; break;
      case DXGI_FORMAT_R32G32B32A32_UINT          : TEXFMT = TEXFMT_A32B32G32R32         ; break;
      case DXGI_FORMAT_A8_UNORM                   : TEXFMT = TEXFMT_A8                   ; break;
#ifdef	W8SUPPORT
      case DXGI_FORMAT_P8_UNORM                   : TEXFMT = TEXFMT_P8                   ; break;
      case DXGI_FORMAT_A8P8_UNORM                 : TEXFMT = TEXFMT_A8P8                 ; break;
#endif

      case DXGI_FORMAT_R9G9B9E5_SHAREDEXP         : TEXFMT = TEXFMT_R9G9B9E5             ; break;

      case DXGI_FORMAT_R8_SNORM                   : TEXFMT = TEXFMT_U8                   ; break;
      case DXGI_FORMAT_R8G8_SNORM                 : TEXFMT = TEXFMT_V8U8                 ; break;
      case DXGI_FORMAT_R8G8B8A8_SNORM             : TEXFMT = TEXFMT_Q8W8V8U8             ; break;
      case DXGI_FORMAT_R16_SNORM                  : TEXFMT = TEXFMT_U16                  ; break;
      case DXGI_FORMAT_R16G16_SNORM               : TEXFMT = TEXFMT_V16U16               ; break;
      case DXGI_FORMAT_R16G16B16A16_SNORM         : TEXFMT = TEXFMT_Q16W16V16U16         ; break;
      case DXGI_FORMAT_R32_SINT                   : break;
      case DXGI_FORMAT_R32G32_SINT                : break;
      case DXGI_FORMAT_R32G32B32_SINT             : break;
      case DXGI_FORMAT_R32G32B32A32_SINT          : break;

      case DXGI_FORMAT_R11G11B10_FLOAT            : TEXFMT = TEXFMT_R11G11B10F           ; break;
      case DXGI_FORMAT_R16_FLOAT                  : TEXFMT = TEXFMT_R16F                 ; break;
      case DXGI_FORMAT_R16G16_FLOAT               : TEXFMT = TEXFMT_G16R16F              ; break;
      case DXGI_FORMAT_R16G16B16A16_FLOAT         : TEXFMT = TEXFMT_A16B16G16R16F        ; break;
      case DXGI_FORMAT_R32_FLOAT                  : TEXFMT = TEXFMT_R32F                 ; break;
      case DXGI_FORMAT_R32G32_FLOAT               : TEXFMT = TEXFMT_G32R32F              ; break;
      case DXGI_FORMAT_R32G32B32_FLOAT            : TEXFMT = TEXFMT_B32G32R32F           ; break;
      case DXGI_FORMAT_R32G32B32A32_FLOAT         : TEXFMT = TEXFMT_A32B32G32R32F        ; break;

      case DXGI_FORMAT_R32G8X24_TYPELESS          : TEXFMT = TEXFMT_R32G8X24             ; break;
      case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS   : TEXFMT = TEXFMT_R32X8X24             ; break;
      case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT    : TEXFMT = TEXFMT_X32G8X24             ; break;
      case DXGI_FORMAT_R24G8_TYPELESS             : TEXFMT = TEXFMT_R24G8                ; break;
      case DXGI_FORMAT_R24_UNORM_X8_TYPELESS      : TEXFMT = TEXFMT_R24X8                ; break;
      case DXGI_FORMAT_X24_TYPELESS_G8_UINT       : TEXFMT = TEXFMT_X24G8                ; break;

      case DXGI_FORMAT_R8G8_B8G8_UNORM            : TEXFMT = TEXFMT_R8G8_B8G8            ; break;
      case DXGI_FORMAT_G8R8_G8B8_UNORM            : TEXFMT = TEXFMT_G8R8_G8B8            ; break;

      case DXGI_FORMAT_BC1_UNORM                  : TEXFMT = TEXFMT_DXT1                 ; break;
      case DXGI_FORMAT_BC2_UNORM                  : TEXFMT = TEXFMT_DXT3                 ; break;
      case DXGI_FORMAT_BC3_UNORM                  : TEXFMT = TEXFMT_DXT5                 ; break;
      case DXGI_FORMAT_BC7_UNORM                  : TEXFMT = TEXFMT_DXTM                 ; break;
      case DXGI_FORMAT_BC4_UNORM                  : TEXFMT = TEXFMT_ATI1                 ; break;
      case DXGI_FORMAT_BC5_UNORM                  : TEXFMT = TEXFMT_ATI2                 ; break;
      case DXGI_FORMAT_BC6H_UF16                  : TEXFMT = TEXFMT_DXTH                 ; break;

      case DXGI_FORMAT_BC1_UNORM_SRGB             : TEXFMT = TEXFMT_DXT1srgb             ; break;
      case DXGI_FORMAT_BC2_UNORM_SRGB             : TEXFMT = TEXFMT_DXT3srgb             ; break;
      case DXGI_FORMAT_BC3_UNORM_SRGB             : TEXFMT = TEXFMT_DXT5srgb             ; break;
      case DXGI_FORMAT_BC7_UNORM_SRGB             : TEXFMT = TEXFMT_DXTMsrgb             ; break;
      case DXGI_FORMAT_BC4_SNORM                  : TEXFMT = TEXFMT_ATI1s                ; break;
      case DXGI_FORMAT_BC5_SNORM                  : TEXFMT = TEXFMT_ATI2s                ; break;
      case DXGI_FORMAT_BC6H_SF16                  : TEXFMT = TEXFMT_DXTHs                ; break;

      case DXGI_FORMAT_D16_UNORM                  : TEXFMT = TEXFMT_D16                  ; break;
      case DXGI_FORMAT_D24_UNORM_S8_UINT          : TEXFMT = TEXFMT_D24S8                ; break;
      case DXGI_FORMAT_D32_FLOAT_S8X24_UINT       : TEXFMT = TEXFMT_D32S8                ; break;
      case DXGI_FORMAT_D32_FLOAT                  : TEXFMT = TEXFMT_D32F_LOCKABLE        ; break;
    }

    return *this;
  }
#endif

  /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */

  static const struct formatID {
    TEXFORMAT fmt;
    const char *name;
    short depth;
    char channels;
    short size;
    bool alpha;
  } formatDatabase[] = {
    { TEXFMT_R1                 , "R1", 1, 1, 1, true },
    { TEXFMT_R8                 , "R8", 8, 1, 8, true },
    { TEXFMT_G8R8               , "G8R8", 8, 2, 16, true },
    { TEXFMT_R8G8B8	        , "R8G8B8", 8, 3, 24, false },
    { TEXFMT_A8R8G8B8           , "A8R8G8B8", 8, 4, 32, true },
    { TEXFMT_X8R8G8B8           , "X8R8G8B8", 8, 4, 32, false },
    { TEXFMT_R5G6B5             , "R5G6B5", 6, 3, 16, false },
    { TEXFMT_X1R5G5B5           , "X1R5G5B5", 5, 3, 16, false },
    { TEXFMT_A1R5G5B5           , "A1R5G5B5", 5, 4, 16, true },

#if	!defined(DX11)
    { TEXFMT_A4R4G4B4           , "A4R4G4B4", 4, 4, 16, true },
    { TEXFMT_X4R4G4B4           , "X4R4G4B4", 4, 3, 16, false },
#elif	defined(W8SUPPORT)
    { TEXFMT_A4R4G4B4           , "A4R4G4B4", 4, 4, 16, true },
    { TEXFMT_X4R4G4B4           , "X4R4G4B4 (in A4R4G4B4)", 4, 3, 16, false },
#else
    { TEXFMT_A4R4G4B4           , "A4R4G4B4 (in R16 typeless)", 4, 4, 16, true },
    { TEXFMT_X4R4G4B4           , "X4R4G4B4 (in R16 typeless)", 4, 3, 16, false },
#endif

    { TEXFMT_R3G3B2             , "R3G3B2", 3, 3, 8, false },
    { TEXFMT_A8R3G3B2           , "A8R3G3B2", 8, 4, 16, true },
    { TEXFMT_A8B8G8R8           , "A8B8G8R8", 8, 4, 32, true },
    { TEXFMT_X8B8G8R8           , "X8B8G8R8", 8, 4, 32, false },
    { TEXFMT_A2B10G10R10        , "A2B10G10R10", 10, 4, 32, true },
    { TEXFMT_A2R10G10B10        , "A2R10G10B10", 10, 4, 32, true },
    { TEXFMT_A2B10G10R10_XR_BIAS, "A2B10G10R10_XR_BIAS", 10, 4, 32, true },
    { TEXFMT_R16                , "R16", 16, 1, 16, false },
    { TEXFMT_G16R16             , "G16R16", 16, 2, 32, false },
    { TEXFMT_A16B16G16R16       , "A16B16G16R16", 16, 4, 64, true },
    { TEXFMT_R32                , "R32", 32, 1, 32, false },
    { TEXFMT_G32R32             , "G32R32", 32, 2, 64, false },
    { TEXFMT_B32G32R32          , "B32G32R32", 32, 3, 96, true },
    { TEXFMT_A32B32G32R32       , "A32B32G32R32", 32, 4, 128, true },

#if	!defined(DX11)
    { TEXFMT_A1                 , "A1", 1, 1, 1, true },
    { TEXFMT_A8                 , "A8", 8, 1, 8, true },
    { TEXFMT_P8                 , "P8", 8, 1, 8, false },
    { TEXFMT_A8P8               , "A8P8", 8, 2, 16, true },
    { TEXFMT_L8                 , "L8", 8, 1, 8, false },
    { TEXFMT_L16                , "L16", 16, 1, 16, false },
    { TEXFMT_A4L4               , "A4L4", 4, 2, 8, true },
    { TEXFMT_A8L8               , "A8L8", 8, 2, 16, true },
#elif	defined(W8SUPPORT)
    { TEXFMT_A1                 , "A1 (in R1)", 1, 1, 1, true },
    { TEXFMT_A8                 , "A8", 8, 1, 8, true },
    { TEXFMT_P8                 , "P8", 8, 1, 8, false },
    { TEXFMT_A8P8               , "A8P8", 8, 2, 16, true },
    { TEXFMT_L8                 , "L8 (in R8)", 8, 1, 8, false },
    { TEXFMT_L16                , "L16 (in R16)", 16, 1, 16, false },
    { TEXFMT_A4L4               , "A4L4 (in R8)", 4, 2, 8, true },
    { TEXFMT_A8L8               , "A8L8 (in G8R8)", 8, 2, 16, true },
#else
    { TEXFMT_A1                 , "A1 (in R1)", 1, 1, 1, true },
    { TEXFMT_A8                 , "A8", 8, 1, 8, true },
    { TEXFMT_P8                 , "P8 (in R8)", 8, 1, 8, false },
    { TEXFMT_A8P8               , "A8P8 (in G8R8)", 8, 2, 16, true },
    { TEXFMT_L8                 , "L8 (in R8)", 8, 1, 8, false },
    { TEXFMT_L16                , "L16 (in R16)", 16, 1, 16, false },
    { TEXFMT_A4L4               , "A4L4 (in R8)", 4, 2, 8, true },
    { TEXFMT_A8L8               , "A8L8 (in G8R8)", 8, 2, 16, true },
#endif

    { TEXFMT_R9G9B9E5           , "R9G9B9E5", 9, 3, 32, true },

    { TEXFMT_U8                 , "U8", 8, 1, 8, false },
    { TEXFMT_V8U8               , "V8U8", 8, 2, 16, false },
    { TEXFMT_CxV8U8             , "CxV8U8", 8, 2, 16, false },
    { TEXFMT_L6V5U5             , "L6V5U5", 6, 3, 16, false },
    { TEXFMT_X8L8V8U8           , "X8L8V8U8", 8, 3, 32, false },
    { TEXFMT_Q8W8V8U8           , "Q8W8V8U8", 8, 4, 32, false },
    { TEXFMT_A2W10V10U10        , "A2W10V10U10", 10, 4, 32, true },
    { TEXFMT_U16                , "U16", 16, 1, 16, false },
    { TEXFMT_V16U16             , "V16U16", 16, 2, 32, false },
    { TEXFMT_Q16W16V16U16       , "Q16W16V16U16", 16, 4, 64, false },

    { TEXFMT_R11G11B10F         , "R11G11B10F", -11, 3, 32, false },
    { TEXFMT_R16F               , "R16F", -10, 1, 16, false },
    { TEXFMT_G16R16F            , "G16R16F", -10, 2, 32, false },
    { TEXFMT_A16B16G16R16F      , "A16B16G16R16F", -10, 4, 64, true },
    { TEXFMT_R32F               , "R32F", -23, 1, 32, false },
    { TEXFMT_G32R32F            , "G32R32F", -23, 2, 64, false },
    { TEXFMT_B32G32R32F         , "B32G32R32F", -23, 3, 96, true },
    { TEXFMT_A32B32G32R32F      , "A32B32G32R32F", -23, 4, 128, true },

    { TEXFMT_R32G8X24           , "R32G8X24", -32, 2, 64, false },
    { TEXFMT_R32X8X24           , "R32X8X24", -32, 2, 64, false },
    { TEXFMT_X32G8X24           , "X32G8X24", -32, 2, 64, false },
    { TEXFMT_R24G8              , "R24G8", 24, 2, 32, false },
    { TEXFMT_R24X8              , "R24X8", 24, 2, 32, false },
    { TEXFMT_X24G8              , "X24G8", 24, 2, 32, false },

    { TEXFMT_UYVY               , "UYVY", 8, 3, (8*4)/(2*2), false },
    { TEXFMT_YUY2               , "YUY2", 8, 3, (8*4)/(2*2), false },
    { TEXFMT_R8G8_B8G8          , "R8G8_B8G8", 8, 3, (8*4)/(2*2), false },
    { TEXFMT_G8R8_G8B8          , "G8R8_G8B8", 8, 3, (8*4)/(2*2), false },

#ifndef DX11
    { TEXFMT_DXT1               , "DXT1", 8, 4, (8*8)/(4*4), true },
    { TEXFMT_DXT2               , "DXT2", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT3               , "DXT3", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT4               , "DXT4", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT5               , "DXT5", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXTM               , "DXTM", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_ATI1               , "ATI1", 8, 1, (8*8)/(4*4), false },
    { TEXFMT_ATI2               , "ATI2", 8, 2, (8*16)/(4*4), false },
    { TEXFMT_DXTH               , "DXTH", -16, 3, (8*16)/(4*4*2), false },

    { TEXFMT_DXT1srgb           , "DXT1 (sRGB)", 8, 4, (8*8)/(4*4), true },
    { TEXFMT_DXT2srgb           , "DXT2 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT3srgb           , "DXT3 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT4srgb           , "DXT4 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT5srgb           , "DXT5 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXTMsrgb           , "DXTM (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_ATI1s              , "ATI1 (signed)", 8, 1, (8*8)/(4*4), false },
    { TEXFMT_ATI2s              , "ATI2 (signed)", 8, 2, (8*16)/(4*4), false },
    { TEXFMT_DXTHs              , "DXTH (signed)", -16, 3, (8*16)/(4*4*2), false },
#else
    { TEXFMT_DXT1               , "BC1", 8, 4, (8*8)/(4*4), true },
    { TEXFMT_DXT2               , "DXT2 (deprecated)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT3               , "BC2", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT4               , "DXT4 (deprecated)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT5               , "BC3", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXTM               , "BC7", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_ATI1               , "BC4", 8, 1, (8*8)/(4*4), false },
    { TEXFMT_ATI2               , "BC5", 8, 2, (8*16)/(4*4), false },
    { TEXFMT_DXTH               , "BC6H", -16, 3, (8*16)/(4*4*2), false },

    { TEXFMT_DXT1srgb           , "BC1 (sRGB)", 8, 4, (8*8)/(4*4), true },
    { TEXFMT_DXT2srgb           , "DXT2 (sRGB, deprecated)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT3srgb           , "BC2 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT4srgb           , "DXT4 (sRGB, deprecated)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXT5srgb           , "BC3 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_DXTMsrgb           , "BC7 (sRGB)", 8, 4, (8*16)/(4*4), true },
    { TEXFMT_ATI1s              , "BC4 (signed)", 8, 1, (8*8)/(4*4), false },
    { TEXFMT_ATI2s              , "BC5 (signed)", 8, 2, (8*16)/(4*4), false },
    { TEXFMT_DXTHs              , "BC6H (signed)", -16, 3, (8*16)/(4*4*2), false },
#endif

    { TEXFMT_S8_LOCKABLE        , "S8_LOCKABLE", 8, 1, 8, false },
    { TEXFMT_D15S1              , "D15S1", 15, 2, 16, false },
    { TEXFMT_D16                , "D16", 16, 1, 16, false },
    { TEXFMT_D16_LOCKABLE       , "D16_LOCKABLE", 16, 1, 16, false },
    { TEXFMT_D24S8              , "D24S8", 24, 2, 32, false },
    { TEXFMT_D24X8              , "D24X8", 24, 1, 32, false },
    { TEXFMT_D24X4S4            , "D24X4S4", 24, 2, 32, false },
    { TEXFMT_D24FS8             , "D24FS8", -23, 2, 32, false },
    { TEXFMT_D32                , "D32", 32, 1, 32, false },
    { TEXFMT_D32_LOCKABLE       , "D32_LOCKABLE", 32, 1, 32, false },
    { TEXFMT_D32S8              , "D32S8", -32, 2, 64, false },
    { TEXFMT_D32F_LOCKABLE      , "D32F_LOCKABLE", -23, 1, 32, false },
    { TEXFMT_INTZ               , "INTZ", 24, 2, 32, false },
    { TEXFMT_DF24               , "DF24", -23, 2, 24, false },
    { TEXFMT_DF16               , "DF16", -10, 2, 16, false },
    { TEXFMT_RAWZ               , "RAWZ", -23, 2, 32, false },

    { TEXFMT_VERTEXDATA         , "VERTEXDATA", 0, 0, 0, false },
    { TEXFMT_INDEX16            , "INDEX16", 16, 1, 16, false },
    { TEXFMT_INDEX32            , "INDEX32", 32, 1, 32, false },
    { TEXFMT_MULTI2_ARGB8       , "MULTI2_ARGB8", 8, 4, 64, true },

    { TEXFMT_BINARYBUFFER       , "BINARYBUFFER", 0, 0, 0, false },
  };

  const char *findFormat(TEXFORMAT fmt) {
    for (int g = 0; g < (sizeof(formatDatabase) / sizeof(formatID)); g++) {
      if (formatDatabase[g].fmt == fmt)
        return formatDatabase[g].name;
    }

    return "unknown";
  }

  short findFormatDepth(TEXFORMAT fmt) {
    for (int g = 0; g < (sizeof(formatDatabase) / sizeof(formatID)); g++) {
      if (formatDatabase[g].fmt == fmt)
        return formatDatabase[g].depth;
    }

    return 0;
  }

  char findFormatChannels(TEXFORMAT fmt) {
    for (int g = 0; g < (sizeof(formatDatabase) / sizeof(formatID)); g++) {
      if (formatDatabase[g].fmt == fmt)
        return formatDatabase[g].channels;
    }

    return 0;
  }

  short findFormatSize(TEXFORMAT fmt) {
    for (int g = 0; g < (sizeof(formatDatabase) / sizeof(formatID)); g++) {
      if (formatDatabase[g].fmt == fmt)
        return formatDatabase[g].size;
    }

    return 0;
  }

  bool findAlpha(TEXFORMAT fmt) {
    for (int g = 0; g < (sizeof(formatDatabase) / sizeof(formatID)); g++) {
      if (formatDatabase[g].fmt == fmt)
        return formatDatabase[g].alpha;
    }

    return 0;
  }

  const char *findFormatRatio(TEXFORMAT fmt, TEXFORMAT before) {
    const char *pfx = findFormat(fmt);
    const char *sfx = NULL;
    static char fstr[256];

    if ((fmt == TEXFMT_DXT1) ||
        (fmt == TEXFMT_DXT2) ||
        (fmt == TEXFMT_DXT3) ||
        (fmt == TEXFMT_DXT4) ||
        (fmt == TEXFMT_DXT5)) {
      pfx = "Optimal DXT";
      if (findAlpha(before) && (before != TEXFMT_DXT1)) {
        if (fmt != TEXFMT_DXT1)
  	  sfx = " (with alpha)";
        else
  	  sfx = " (strip alpha)";
      }
    }

    if ((fmt == TEXFMT_ATI1)) {
      pfx = "Partial DXT";
      if (findAlpha(before) && (before != TEXFMT_DXT1))
	sfx = " (strip XY & alpha)";
      else
	sfx = " (strip XY)";
    }

    if ((fmt == TEXFMT_ATI2)) {
      pfx = "Partial DXT";
      if (findAlpha(before) && (before != TEXFMT_DXT1))
        sfx = " (strip Z & alpha)";
      else
        sfx = " (strip Z)";
    }

    if ((fmt == TEXFMT_A2R10G10B10)) {
      if (!findAlpha(before)) {
        pfx = "X2R10G10B10";
      }
    }

    short f = findFormatSize(fmt);
    short g = findFormatSize(before);
    short a = f, b = g, c;

    while (1) {
      a = a % b;
      if (a == 0) {
        c = b; break; }

      b = b % a;
      if (b == 0) {
        c = a; break; }
    }

    f /= c;
    g /= c;

    if (sfx)
      sprintf(fstr, "DDS - %s%s, %d:%d", pfx, sfx, f, g);
    else
      sprintf(fstr, "DDS - %s, %d:%d", pfx, f, g);

    return fstr;
  }

} // namespace squash
