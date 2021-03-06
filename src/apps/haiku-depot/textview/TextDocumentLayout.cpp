/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TextDocumentLayout.h"

#include <new>
#include <stdio.h>

#include <View.h>


TextDocumentLayout::TextDocumentLayout()
	:
	fWidth(0.0f),
	fLayoutValid(false),

	fDocument(),
	fParagraphLayouts()
{
}


TextDocumentLayout::TextDocumentLayout(const TextDocumentRef& document)
	:
	fWidth(0.0f),
	fLayoutValid(false),

	fDocument(document),
	fParagraphLayouts()
{
	_Init();
}


TextDocumentLayout::TextDocumentLayout(const TextDocumentLayout& other)
	:
	fWidth(other.fWidth),
	fLayoutValid(other.fLayoutValid),

	fDocument(other.fDocument),
	fParagraphLayouts(other.fParagraphLayouts)
{
}


TextDocumentLayout::~TextDocumentLayout()
{
}


void
TextDocumentLayout::SetTextDocument(const TextDocumentRef& document)
{
	if (fDocument != document) {
		fDocument = document;
		_Init();
		fLayoutValid = false;
	}
}


void
TextDocumentLayout::SetWidth(float width)
{
	if (fWidth != width) {
		fWidth = width;
		fLayoutValid = false;
	}
}


float
TextDocumentLayout::Height()
{
	_ValidateLayout();

	float height = 0.0f;

	if (fParagraphLayouts.CountItems() > 0) {
		const ParagraphLayoutInfo& lastLayout = fParagraphLayouts.LastItem();
		height = lastLayout.y + lastLayout.layout->Height();
	}

	return height;
}


void
TextDocumentLayout::Draw(BView* view, const BPoint& offset,
	const BRect& updateRect)
{
	_ValidateLayout();

	int layoutCount = fParagraphLayouts.CountItems();
	for (int i = 0; i < layoutCount; i++) {
		const ParagraphLayoutInfo& layout = fParagraphLayouts.ItemAtFast(i);
		BPoint location(offset.x, offset.y + layout.y);
		if (location.y > updateRect.bottom)
			break;
		if (location.y + layout.layout->Height() > updateRect.top)
			layout.layout->Draw(view, location);
	}
}


int32
TextDocumentLayout::LineIndexForOffset(int32 textOffset)
{
	int32 index = _ParagraphLayoutIndexForOffset(textOffset);
	if (index >= 0) {
		int32 lineIndex = 0;
		for (int32 i = 0; i < index; i++) {
			lineIndex += fParagraphLayouts.ItemAtFast(i).layout->CountLines();
		}
	
		const ParagraphLayoutInfo& info = fParagraphLayouts.ItemAtFast(index);
		return lineIndex + info.layout->LineIndexForOffset(textOffset);
	}
	
	return 0;
}


void
TextDocumentLayout::GetTextBounds(int32 textOffset, float& x1, float& y1,
	float& x2, float& y2)
{
	int32 index = _ParagraphLayoutIndexForOffset(textOffset);
	if (index >= 0) {
		const ParagraphLayoutInfo& info = fParagraphLayouts.ItemAtFast(index);
		info.layout->GetTextBounds(textOffset, x1, y1, x2, y2);
		y1 += info.y;
		y2 += info.y;
		return;
	}

	x1 = 0.0f;
	y1 = 0.0f;
	x2 = 0.0f;
	y2 = 0.0f;
}


int32
TextDocumentLayout::TextOffsetAt(float x, float y, bool& rightOfCenter)
{
	_ValidateLayout();

	int32 textOffset = 0;
	rightOfCenter = false;

	int32 paragraphs = fParagraphLayouts.CountItems();
	for (int32 i = 0; i < paragraphs; i++) {
		const ParagraphLayoutInfo& info = fParagraphLayouts.ItemAtFast(i);
		if (y > info.y + info.layout->Height()) {
			textOffset += info.layout->CountGlyphs();
			continue;
		}

		textOffset += info.layout->TextOffsetAt(x, y - info.y, rightOfCenter);
		break;
	}
	
	return textOffset;
}

// #pragma mark - private


void
TextDocumentLayout::_ValidateLayout()
{
	if (!fLayoutValid) {
		_Layout();
		fLayoutValid = true;
	}
}


void
TextDocumentLayout::_Init()
{
	fParagraphLayouts.Clear();

	const ParagraphList& paragraphs = fDocument->Paragraphs();

	int paragraphCount = paragraphs.CountItems();
	for (int i = 0; i < paragraphCount; i++) {
		const Paragraph& paragraph = paragraphs.ItemAtFast(i);
		ParagraphLayoutRef layout(new(std::nothrow) ParagraphLayout(paragraph),
			true);
		if (layout.Get() == NULL
			|| !fParagraphLayouts.Add(ParagraphLayoutInfo(0.0f, layout))) {
			fprintf(stderr, "TextDocumentLayout::_Layout() - out of memory\n");
			return;
		}
	}
}


void
TextDocumentLayout::_Layout()
{
	float y = 0.0f;

	int layoutCount = fParagraphLayouts.CountItems();
	for (int i = 0; i < layoutCount; i++) {
		ParagraphLayoutInfo info = fParagraphLayouts.ItemAtFast(i);
		const ParagraphStyle& style = info.layout->Style();

		if (i > 0)
			y += style.SpacingTop();

		fParagraphLayouts.Replace(i, ParagraphLayoutInfo(y, info.layout));

		info.layout->SetWidth(fWidth);
		y += info.layout->Height() + style.SpacingBottom();
	}
}


int32
TextDocumentLayout::_ParagraphLayoutIndexForOffset(int32& textOffset)
{
	_ValidateLayout();

	int32 paragraphs = fParagraphLayouts.CountItems();
	for (int32 i = 0; i < paragraphs; i++) {
		const ParagraphLayoutInfo& info = fParagraphLayouts.ItemAtFast(i);
		
		int32 length = info.layout->CountGlyphs();
		if (textOffset > length) {
			textOffset -= length;
			continue;
		}
		
		return i;
	}
	
	return -1;
}

