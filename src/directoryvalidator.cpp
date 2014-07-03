#include "directoryvalidator.h"
#include <QDir>
#include <QDebug>


//==================================================================================================
// Public:

DirectoryValidator::DirectoryValidator( QObject* parent ) :
    QValidator( parent )
{
}



QValidator::State DirectoryValidator::validate( QString& input, int& pos ) const
{
    QDir dir( input );
    State state;

    if ( dir.exists() )
    {
        state = Acceptable;
        emit isValid( true );
    }
    else
    {
        state = Intermediate;
        emit isValid( false );
    }

    return state;
}



void DirectoryValidator::fixup( QString& input ) const
{
    QDir::cleanPath( input );
}
