/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2007, 2010 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef HTMLMarqueeElement_h
#define HTMLMarqueeElement_h

#include "core/html/HTMLElement.h"

namespace blink {

class HTMLMarqueeElement final : public HTMLElement {
    DEFINE_WRAPPERTYPEINFO();
public:
    static HTMLMarqueeElement* create(Document&);

    void attributeChanged(const QualifiedName&, const AtomicString& oldValue, const AtomicString& newValue, AttributeModificationReason) final;
    InsertionNotificationRequest insertedInto(ContainerNode*) final;
    void removedFrom(ContainerNode*) final;

    bool isHorizontal() const;

private:
    explicit HTMLMarqueeElement(Document&);
};

} // namespace blink

#endif // HTMLMarqueeElement_h
