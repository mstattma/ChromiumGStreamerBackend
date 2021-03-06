// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/slider.h"

#include <algorithm>

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/accessibility/ax_view_state.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/events/event.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/resources/grit/ui_resources.h"
#include "ui/views/controls/slider.h"
#include "ui/views/resources/grit/views_resources.h"
#include "ui/views/widget/widget.h"

namespace {
const int kSlideValueChangeDurationMS = 150;

const int kBarImagesActive[] = {
    IDR_SLIDER_ACTIVE_LEFT,
    IDR_SLIDER_ACTIVE_CENTER,
    IDR_SLIDER_PRESSED_CENTER,
    IDR_SLIDER_PRESSED_RIGHT,
};

const int kBarImagesDisabled[] = {
    IDR_SLIDER_DISABLED_LEFT,
    IDR_SLIDER_DISABLED_CENTER,
    IDR_SLIDER_DISABLED_CENTER,
    IDR_SLIDER_DISABLED_RIGHT,
};

// The image chunks.
enum BorderElements {
  LEFT,
  CENTER_LEFT,
  CENTER_RIGHT,
  RIGHT,
};
}  // namespace

namespace views {

// static
const char Slider::kViewClassName[] = "Slider";

Slider::Slider(SliderListener* listener)
    : listener_(listener),
      value_(0.f),
      keyboard_increment_(0.1f),
      animating_value_(0.f),
      value_is_valid_(false),
      accessibility_events_enabled_(true),
      focus_border_color_(0),
      initial_button_offset_(0),
      bar_active_images_(kBarImagesActive),
      bar_disabled_images_(kBarImagesDisabled) {
  EnableCanvasFlippingForRTLUI(true);
#if defined(OS_MACOSX)
  SetFocusBehavior(FocusBehavior::ACCESSIBLE_ONLY);
#else
  SetFocusBehavior(FocusBehavior::ALWAYS);
#endif

  UpdateState(true);
}

Slider::~Slider() {
}

void Slider::SetValue(float value) {
  SetValueInternal(value, VALUE_CHANGED_BY_API);
}

void Slider::SetValueInternal(float value, SliderChangeReason reason) {
  bool old_value_valid = value_is_valid_;

  value_is_valid_ = true;
  if (value < 0.0)
    value = 0.0;
  else if (value > 1.0)
    value = 1.0;
  if (value_ == value)
    return;
  float old_value = value_;
  value_ = value;
  if (listener_)
    listener_->SliderValueChanged(this, value_, old_value, reason);

  if (old_value_valid && base::MessageLoop::current()) {
    // Do not animate when setting the value of the slider for the first time.
    // There is no message-loop when running tests. So we cannot animate then.
    animating_value_ = old_value;
    move_animation_.reset(new gfx::SlideAnimation(this));
    move_animation_->SetSlideDuration(kSlideValueChangeDurationMS);
    move_animation_->Show();
    AnimationProgressed(move_animation_.get());
  } else {
    SchedulePaint();
  }
  if (accessibility_events_enabled_ && GetWidget()) {
    NotifyAccessibilityEvent(
        ui::AX_EVENT_VALUE_CHANGED, true);
  }
}

void Slider::PrepareForMove(const int new_x) {
  // Try to remember the position of the mouse cursor on the button.
  gfx::Insets inset = GetInsets();
  gfx::Rect content = GetContentsBounds();
  float value = move_animation_.get() && move_animation_->is_animating() ?
        animating_value_ : value_;

  const int thumb_x = value * (content.width() - thumb_->width());
  const int candidate_x = (base::i18n::IsRTL() ?
      width() - (new_x - inset.left()) :
      new_x - inset.left()) - thumb_x;
  if (candidate_x >= 0 && candidate_x < thumb_->width())
    initial_button_offset_ = candidate_x;
  else
    initial_button_offset_ = thumb_->width() / 2;
}

void Slider::MoveButtonTo(const gfx::Point& point) {
  gfx::Insets inset = GetInsets();
  // Calculate the value.
  int amount = base::i18n::IsRTL()
                   ? width() - inset.left() - point.x() - initial_button_offset_
                   : point.x() - inset.left() - initial_button_offset_;
  SetValueInternal(
      static_cast<float>(amount) / (width() - inset.width() - thumb_->width()),
      VALUE_CHANGED_BY_USER);
}

void Slider::UpdateState(bool control_on) {
  ResourceBundle& rb = ResourceBundle::GetSharedInstance();
  if (control_on) {
    thumb_ = rb.GetImageNamed(IDR_SLIDER_ACTIVE_THUMB).ToImageSkia();
    for (int i = 0; i < 4; ++i)
      images_[i] = rb.GetImageNamed(bar_active_images_[i]).ToImageSkia();
  } else {
    thumb_ = rb.GetImageNamed(IDR_SLIDER_DISABLED_THUMB).ToImageSkia();
    for (int i = 0; i < 4; ++i)
      images_[i] = rb.GetImageNamed(bar_disabled_images_[i]).ToImageSkia();
  }
  bar_height_ = images_[LEFT]->height();
  SchedulePaint();
}

void Slider::SetAccessibleName(const base::string16& name) {
  accessible_name_ = name;
}

void Slider::OnPaintFocus(gfx::Canvas* canvas) {
  if (!HasFocus())
    return;

  if (!focus_border_color_) {
    canvas->DrawFocusRect(GetLocalBounds());
  } else if (HasFocus()) {
    canvas->DrawSolidFocusRect(
        gfx::Rect(1, 1, width() - 3, height() - 3),
        focus_border_color_);
  }
}

const char* Slider::GetClassName() const {
  return kViewClassName;
}

gfx::Size Slider::GetPreferredSize() const {
  const int kSizeMajor = 200;
  const int kSizeMinor = 40;

  return gfx::Size(std::max(width(), kSizeMajor), kSizeMinor);
}

void Slider::OnPaint(gfx::Canvas* canvas) {
  View::OnPaint(canvas);
  gfx::Rect content = GetContentsBounds();
  float value = move_animation_.get() && move_animation_->is_animating() ?
      animating_value_ : value_;
  // Paint slider bar with image resources.

  // Inset the slider bar a little bit, so that the left or the right end of
  // the slider bar will not be exposed under the thumb button when the thumb
  // button slides to the left most or right most position.
  const int kBarInsetX = 2;
  int bar_width = content.width() - kBarInsetX * 2;
  int bar_cy = content.height() / 2 - bar_height_ / 2;

  int w = content.width() - thumb_->width();
  int full = value * w;
  int middle = std::max(full, images_[LEFT]->width());

  canvas->Save();
  canvas->Translate(gfx::Vector2d(kBarInsetX, bar_cy));
  canvas->DrawImageInt(*images_[LEFT], 0, 0);
  canvas->DrawImageInt(*images_[RIGHT], bar_width - images_[RIGHT]->width(), 0);
  canvas->TileImageInt(*images_[CENTER_LEFT], images_[LEFT]->width(), 0,
                       middle - images_[LEFT]->width(), bar_height_);
  canvas->TileImageInt(*images_[CENTER_RIGHT], middle, 0,
                       bar_width - middle - images_[RIGHT]->width(),
                       bar_height_);
  canvas->Restore();

  // Paint slider thumb.
  int button_cx = content.x() + full;
  int thumb_y = content.height() / 2 - thumb_->height() / 2;
  canvas->DrawImageInt(*thumb_, button_cx, thumb_y);
  OnPaintFocus(canvas);
}

bool Slider::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.IsOnlyLeftMouseButton())
    return false;
  OnSliderDragStarted();
  PrepareForMove(event.location().x());
  MoveButtonTo(event.location());
  return true;
}

bool Slider::OnMouseDragged(const ui::MouseEvent& event) {
  MoveButtonTo(event.location());
  return true;
}

void Slider::OnMouseReleased(const ui::MouseEvent& event) {
  OnSliderDragEnded();
}

bool Slider::OnKeyPressed(const ui::KeyEvent& event) {
  float new_value = value_;
  if (event.key_code() == ui::VKEY_LEFT)
    new_value -= keyboard_increment_;
  else if (event.key_code() == ui::VKEY_RIGHT)
    new_value += keyboard_increment_;
  else
    return false;
  SetValueInternal(new_value, VALUE_CHANGED_BY_USER);
  return true;
}

void Slider::OnFocus() {
  View::OnFocus();
  SchedulePaint();
}

void Slider::OnBlur() {
  View::OnBlur();
  SchedulePaint();
}

void Slider::OnGestureEvent(ui::GestureEvent* event) {
  switch (event->type()) {
    // In a multi point gesture only the touch point will generate
    // an ET_GESTURE_TAP_DOWN event.
    case ui::ET_GESTURE_TAP_DOWN:
      OnSliderDragStarted();
      PrepareForMove(event->location().x());
      // Intentional fall through to next case.
    case ui::ET_GESTURE_SCROLL_BEGIN:
    case ui::ET_GESTURE_SCROLL_UPDATE:
      MoveButtonTo(event->location());
      event->SetHandled();
      break;
    case ui::ET_GESTURE_END:
      MoveButtonTo(event->location());
      event->SetHandled();
      if (event->details().touch_points() <= 1)
        OnSliderDragEnded();
      break;
    default:
      break;
  }
}

void Slider::AnimationProgressed(const gfx::Animation* animation) {
  animating_value_ = animation->CurrentValueBetween(animating_value_, value_);
  SchedulePaint();
}

void Slider::GetAccessibleState(ui::AXViewState* state) {
  state->role = ui::AX_ROLE_SLIDER;
  state->name = accessible_name_;
  state->value = base::UTF8ToUTF16(
      base::StringPrintf("%d%%", static_cast<int>(value_ * 100 + 0.5)));
}

void Slider::OnSliderDragStarted() {
  if (listener_)
    listener_->SliderDragStarted(this);
}

void Slider::OnSliderDragEnded() {
  if (listener_)
    listener_->SliderDragEnded(this);
}

}  // namespace views
