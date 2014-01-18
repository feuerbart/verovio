/////////////////////////////////////////////////////////////////////////////
// Name:        measure.h
// Author:      Laurent Pugin
// Created:     2012
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#include "measure.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "vrv.h"
#include "vrvdef.h"
#include "io.h"
#include "page.h"
#include "staff.h"
#include "system.h"

namespace vrv {

//----------------------------------------------------------------------------
// Measure
//----------------------------------------------------------------------------

Measure::Measure( bool measureMusic, int logMeasureNb ):
    DocObject("measure-")
{
	Clear( );
    m_measuredMusic = measureMusic;
    m_logMeasureNb = logMeasureNb;
    if ( !m_measuredMusic ) {
        m_xAbs = 0;
    }
}

Measure::~Measure()
{
    
}

void Measure::Clear()
{
	ClearChildren();
    m_parent = NULL;
    m_logMeasureNb = -1;
    m_measuredMusic = true;
    m_xAbs = VRV_UNSET;
    m_xRel = 0;
    m_xDrawing = 0;
    // by default, we have a single barline on the right (none on the left)
    m_rightBarline.m_barlineType = BARLINE_SINGLE;
    m_leftBarline.m_barlineType = BARLINE_NONE;
}

int Measure::Save( ArrayPtrVoid params )
{
    // param 0: output stream
    FileOutputStream *output = (FileOutputStream*)params[0];
    if (!output->WriteMeasure( this )) {
        return FUNCTOR_STOP;
    }
    return FUNCTOR_CONTINUE;

}

void Measure::AddStaff( Staff *staff )
{    
	staff->SetParent( this );
	m_children.push_back( staff );
    
    if ( staff->GetStaffNo() == -1 ) {
        staff->SetStaffNo( this->GetStaffCount() );
    }
}

Staff *Measure::GetFirst( )
{
	if ( m_children.empty() )
		return NULL;
	return (Staff*)m_children[0];
}

Staff *Measure::GetLast( )
{
	if ( m_children.empty() )
		return NULL;
	int i = GetStaffCount() - 1;
	return (Staff*)m_children[i];
}

Staff *Measure::GetNext( Staff *staff )
{
    if ( !staff || m_children.empty())
        return NULL;
    
	int i = 0; GetChildIndex( staff );
    
	if ((i == -1 ) || ( i >= GetStaffCount() - 1 ))
		return NULL;
    
	return (Staff*)m_children[i + 1];
}

Staff *Measure::GetPrevious( Staff *staff )
{
    if ( !staff || m_children.empty())
        return NULL;
    
	int i = GetChildIndex( staff );
    
	if ((i == -1 ) || ( i <= 0 ))
        return NULL;
	
    return (Staff*)m_children[i - 1];
}


Staff *Measure::GetStaffWithIdx( int staffIdx )
{
    if ( staffIdx > (int)m_children.size() - 1 )
        return NULL;
	
	return (Staff*)m_children[staffIdx];
}


Staff *Measure::GetStaffWithNo( int staffNo )
{
    int i;
    Staff *staff = NULL;
    for (i = 0; i < this->GetStaffCount(); i++ ) {
        staff = dynamic_cast<Staff*>(m_children[i]);
        if ( staff && (staff->GetStaffNo() == staffNo ) ) {
            return staff;
        }
    }
	return NULL;
}
 

void Measure::ResetDrawingValues()
{
    m_xRel = 0;
    m_xDrawing = 0;
    if ( m_measureAligner.GetLeftAlignment() ) {
        m_measureAligner.GetLeftAlignment()->SetXRel( 0 );
    }
    if ( m_measureAligner.GetRightAlignment() ) {
        m_measureAligner.GetRightAlignment()->SetXRel( 0 );
    }
}


int Measure::GetXRel()
{
    if ( m_measureAligner.GetLeftAlignment() ) {
        return m_measureAligner.GetLeftAlignment()->GetXRel();
    }
    return 0;
}

int Measure::GetXRelRight()
{
    if ( m_measureAligner.GetRightAlignment() ) {
        return m_measureAligner.GetRightAlignment()->GetXRel();
    }
    return 0;
}

//----------------------------------------------------------------------------
// Measure functor methods
//----------------------------------------------------------------------------

int Measure::Align( ArrayPtrVoid params )
{
    // param 0: the measureAligner
    // param 1: the time (unused)
    // param 2: the systemAligner (unused)
    // param 3: the staffNb
    MeasureAligner **measureAligner = (MeasureAligner**)params[0];
    int *staffNb = (int*)params[3];
    
    // clear the content of the measureAligner
    m_measureAligner.Reset();
    
    // point to it
    (*measureAligner) = &m_measureAligner;
    
    if ( m_rightBarline.m_barlineType != BARLINE_NONE ) {
        m_rightBarline.SetAlignment( m_measureAligner.GetRightAlignment() );
    }
    
    // we also need to reset the staffNb
    (*staffNb) = 0;
    
    assert( *measureAligner );
        
    return FUNCTOR_CONTINUE;
}

int Measure::IntegrateBoundingBoxXShift( ArrayPtrVoid params )
{
    // param 0: the cumulated shift (unused)
    // param 1: the functor to be redirected to Aligner
    MusFunctor *integrateBoundingBoxShift = (MusFunctor*)params[1];
    
    m_measureAligner.Process( integrateBoundingBoxShift, params);
    
    return FUNCTOR_SIBLINGS;
}

int Measure::SetAligmentXPos( ArrayPtrVoid params )
{
    // param 0: the previous time position (unused)
    // param 1: the previous x rel position (unused)
    // param 2: the functor to be redirected to Aligner
    MusFunctor *setAligmnentPosX = (MusFunctor*)params[2];
    
    m_measureAligner.Process( setAligmnentPosX, params);
    
    return FUNCTOR_SIBLINGS;
}

#include <math.h>

int Measure::JustifyX( ArrayPtrVoid params )
{
    // param 0: the justification ratio
    // param 1: the system full width (without system margins) (unused)
    // param 2: the functor to be redirected to the MeasureAligner
    double *ratio = (double*)params[0];
    MusFunctor *justifyX = (MusFunctor*)params[2];
    
    this->m_xRel = ceil((*ratio) * (double)this->m_xRel);
    
    m_measureAligner.Process( justifyX, params );
    
    return FUNCTOR_SIBLINGS;
}


int Measure::AlignMeasures( ArrayPtrVoid params )
{
    // param 0: the cumulated shift
    int *shift = (int*)params[0];
    
    this->m_xRel = (*shift);
    
    assert( m_measureAligner.GetRightAlignment() );
    
    (*shift) += m_measureAligner.GetRightAlignment()->GetXRel();
    
    // We also need to take into account the measure end (right) barline with here
    if (GetRightBarlineType() != BARLINE_NONE) {
        // shift the next measure of the total with
        (*shift) += GetRightBarline()->GetAlignment()->GetMaxWidth();
    }
    
    return FUNCTOR_SIBLINGS;
}
    
int Measure::CastOffSystems( ArrayPtrVoid params )
{
    // param 0: a pointer to the system we are taking the content from
    // param 1: a pointer the page we are adding system to
    // param 2: a pointer to the current system
    // param 3: the cummulated shift (m_xRel of the first measure of the current system)
    // param 4: the system width
    System *contentSystem = (System*)params[0];
    Page *page = (Page*)params[1];
    System **currentSystem = (System**)params[2];
    int *shift = (int*)params[3];
    int *systemWidth = (int*)params[4];
    
    if ( ( (*currentSystem)->GetChildCount() > 0 ) && ( this->m_xRel + this->GetXRelRight() - (*shift) > (*systemWidth) ) ) {
        (*currentSystem) = new System();
        page->AddSystem( *currentSystem );
        (*shift) = this->m_xRel;;
    }
    
    // Special case where we use the Relinquish method.
    // We want to move the measure to the currentSystem. However, we cannot use DetachChild
    // from the content System because this screws up the iterator. Relinquish gives up
    // the ownership of the Measure - the contentSystem will be deleted afterwards.
    Measure *measure = dynamic_cast<Measure*>( contentSystem->Relinquish( this->GetIdx()) );
    (*currentSystem)->AddMeasure( measure );
    
    return FUNCTOR_SIBLINGS;
}

} // namespace vrv