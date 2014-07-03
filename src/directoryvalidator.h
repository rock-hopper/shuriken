#ifndef DIRECTORYVALIDATOR_H
#define DIRECTORYVALIDATOR_H

#include <QValidator>
#include "JuceHeader.h"


class DirectoryValidator : public QValidator
{
    Q_OBJECT

public:
    DirectoryValidator( QObject* parent = NULL );

    State validate( QString& input, int& pos ) const;
    void fixup( QString& input ) const;

signals:
    void isValid( const bool isValid ) const;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( DirectoryValidator );
};

#endif // DIRECTORYVALIDATOR_H
