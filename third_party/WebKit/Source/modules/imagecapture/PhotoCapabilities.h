// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PhotoCapabilities_h
#define PhotoCapabilities_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "media/mojo/interfaces/image_capture.mojom-blink.h"
#include "modules/imagecapture/MediaSettingsRange.h"
#include "wtf/text/WTFString.h"

namespace blink {

class PhotoCapabilities final
    : public GarbageCollectedFinalized<PhotoCapabilities>
    , public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PhotoCapabilities* create();
    virtual ~PhotoCapabilities() = default;

    MediaSettingsRange* iso() const { return m_iso; }
    void setIso(MediaSettingsRange* value) { m_iso = value; }

    MediaSettingsRange* imageHeight() const { return m_imageHeight; }
    void setImageHeight(MediaSettingsRange* value) { m_imageHeight = value; }

    MediaSettingsRange* imageWidth() const { return m_imageWidth; }
    void setImageWidth(MediaSettingsRange* value) { m_imageWidth = value; }

    MediaSettingsRange* zoom() const { return m_zoom; }
    void setZoom(MediaSettingsRange* value) { m_zoom = value; }

    String focusMode() const;
    void setFocusMode(media::mojom::blink::MeteringMode focusMode) { m_focusMode = focusMode; }

    String exposureMode() const;
    void setExposureMode(media::mojom::blink::MeteringMode exposureMode) { m_exposureMode = exposureMode; }

    MediaSettingsRange* exposureCompensation() const { return m_exposureCompensation; }
    void setExposureCompensation(MediaSettingsRange* value) { m_exposureCompensation = value; }

    String whiteBalanceMode() const;
    void setWhiteBalanceMode(media::mojom::blink::MeteringMode whiteBalanceMode) { m_whiteBalanceMode = whiteBalanceMode; }

    String fillLightMode() const;
    void setFillLightMode(media::mojom::blink::FillLightMode fillLightMode) { m_fillLightMode = fillLightMode; }

    DECLARE_VIRTUAL_TRACE();

private:
    PhotoCapabilities() = default;

    Member<MediaSettingsRange> m_iso;
    Member<MediaSettingsRange> m_imageHeight;
    Member<MediaSettingsRange> m_imageWidth;
    Member<MediaSettingsRange> m_zoom;
    media::mojom::blink::MeteringMode m_focusMode = media::mojom::blink::MeteringMode::NONE;
    media::mojom::blink::MeteringMode m_exposureMode = media::mojom::blink::MeteringMode::NONE;
    Member<MediaSettingsRange> m_exposureCompensation;
    media::mojom::blink::MeteringMode m_whiteBalanceMode = media::mojom::blink::MeteringMode::NONE;
    media::mojom::blink::FillLightMode m_fillLightMode = media::mojom::blink::FillLightMode::NONE;
};

} // namespace blink

#endif // PhotoCapabilities_h
