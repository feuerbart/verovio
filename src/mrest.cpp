/////////////////////////////////////////////////////////////////////////////
// Name:        mrest.cpp
// Author:      Laurent Pugin
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "mrest.h"

namespace vrv {

//----------------------------------------------------------------------------
// MRest
//----------------------------------------------------------------------------

MRest::MRest() : LayerElement("mrest-"), AttVisibility()
{
    RegisterAttClass(ATT_VISIBILITY);

    Reset();
}

MRest::~MRest()
{
}

void MRest::Reset()
{
    LayerElement::Reset();
    ResetVisibility();
}

} // namespace vrv
