/*
 * Copyright (C) 2009, 2010, 2011 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (C) 2019 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * klogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with klogg.  If not, see <http://www.gnu.org/licenses/>.
 */

// This file implements classes Highlighter and HighlighterSet

#include <QSettings>

#include "highlighterset.h"
#include "log.h"
#include "uuid.h"

QRegularExpression::PatternOptions getPatternOptions( bool ignoreCase )
{
    QRegularExpression::PatternOptions options = QRegularExpression::UseUnicodePropertiesOption;

    if ( ignoreCase ) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    return options;
}

Highlighter::Highlighter( const QString& pattern, bool ignoreCase, bool onlyMatch,
                          const QColor& foreColor, const QColor& backColor )
    : regexp_( pattern, getPatternOptions( ignoreCase ) )
    , highlightOnlyMatch_( onlyMatch )
    , foreColor_( foreColor )
    , backColor_( backColor )
{
    LOG_DEBUG << "New Highlighter, fore: " << foreColor_.name() << " back: " << backColor_.name();
}

QString Highlighter::pattern() const
{
    return regexp_.pattern();
}

void Highlighter::setPattern( const QString& pattern )
{
    regexp_.setPattern( pattern );
}

bool Highlighter::ignoreCase() const
{
    return regexp_.patternOptions().testFlag( QRegularExpression::CaseInsensitiveOption );
}

void Highlighter::setIgnoreCase( bool ignoreCase )
{
    regexp_.setPatternOptions( getPatternOptions( ignoreCase ) );
}

bool Highlighter::useRegex() const
{
    return useRegex_;
}

void Highlighter::setUseRegex( bool useRegex )
{
    useRegex_ = useRegex;
}

bool Highlighter::highlightOnlyMatch() const
{
    return highlightOnlyMatch_;
}

void Highlighter::setHighlightOnlyMatch( bool onlyMatch )
{
    highlightOnlyMatch_ = onlyMatch;
}

const QColor& Highlighter::foreColor() const
{
    return foreColor_;
}

void Highlighter::setForeColor( const QColor& foreColor )
{
    foreColor_ = foreColor;
}

const QColor& Highlighter::backColor() const
{
    return backColor_;
}

void Highlighter::setBackColor( const QColor& backColor )
{
    backColor_ = backColor;
}

bool Highlighter::matchLine( const QString& line, std::vector<HighlightedMatch>& matches ) const
{
    matches.clear();

    const auto pattern
        = useRegex_ ? regexp_.pattern() : QRegularExpression::escape( regexp_.pattern() );

    const auto matchingRegex = QRegularExpression( pattern, regexp_.patternOptions() );

    QRegularExpressionMatchIterator matchIterator = matchingRegex.globalMatch( line );

    while ( matchIterator.hasNext() ) {
        QRegularExpressionMatch match = matchIterator.next();
        if ( matchingRegex.captureCount() > 0 ) {
            for ( int i = 1; i <= match.lastCapturedIndex(); ++i ) {
                matches.emplace_back( match.capturedStart( i ), match.capturedLength( i ),
                                      foreColor_, backColor_ );
            }
        }
        else {
            matches.emplace_back( match.capturedStart( 0 ), match.capturedLength( 0 ), foreColor_,
                                  backColor_ );
        }
    }

    return ( !matches.empty() );
}

HighlighterSet HighlighterSet::createNewSet( const QString& name )
{
    return HighlighterSet{ name };
}

HighlighterSet::HighlighterSet( const QString& name )
    : name_( name )
    , id_( generateIdFromUuid() )
{
}

QString HighlighterSet::name() const
{
    return name_;
}

QString HighlighterSet::id() const
{
    return id_;
}

bool HighlighterSet::isEmpty() const
{
    return highlighterList_.isEmpty();
}

HighlighterMatchType HighlighterSet::matchLine( const QString& line,
                                                std::vector<HighlightedMatch>& matches ) const
{
    auto matchType = HighlighterMatchType::NoMatch;
    for ( auto hl = highlighterList_.rbegin(); hl != highlighterList_.rend(); ++hl ) {
        std::vector<HighlightedMatch> thisMatches;
        if ( !hl->matchLine( line, thisMatches ) ) {
            continue;
        }

        if ( !hl->highlightOnlyMatch() ) {
            matchType = HighlighterMatchType::LineMatch;

            matches.clear();
            matches.emplace_back( 0, line.length(), hl->foreColor(), hl->backColor() );
        }
        else {
            if ( matchType != HighlighterMatchType::LineMatch ) {
                matchType = HighlighterMatchType::WordMatch;
            }

            matches.insert( matches.end(), std::make_move_iterator( thisMatches.begin() ),
                            std::make_move_iterator( thisMatches.end() ) );
        }
    }

    return matchType;
}

//
// Persistable virtual functions implementation
//

void Highlighter::saveToStorage( QSettings& settings ) const
{
    LOG_DEBUG << "Highlighter::saveToStorage";

    settings.setValue( "regexp", regexp_.pattern() );
    settings.setValue( "ignore_case", regexp_.patternOptions().testFlag(
                                          QRegularExpression::CaseInsensitiveOption ) );
    settings.setValue( "match_only", highlightOnlyMatch_ );
    settings.setValue( "use_regex", useRegex_ );
    // save colors as user friendly strings in config
    settings.setValue( "fore_colour", foreColor_.name() );
    settings.setValue( "back_colour", backColor_.name() );
}

void Highlighter::retrieveFromStorage( QSettings& settings )
{
    LOG_DEBUG << "Highlighter::retrieveFromStorage";

    regexp_ = QRegularExpression(
        settings.value( "regexp" ).toString(),
        getPatternOptions( settings.value( "ignore_case", false ).toBool() ) );
    highlightOnlyMatch_ = settings.value( "match_only", false ).toBool();
    useRegex_ = settings.value( "use_regex", true ).toBool();
    foreColor_ = QColor( settings.value( "fore_colour" ).toString() );
    backColor_ = QColor( settings.value( "back_colour" ).toString() );
}

void HighlighterSet::saveToStorage( QSettings& settings ) const
{
    LOG_DEBUG << "HighlighterSet::saveToStorage";

    settings.beginGroup( "HighlighterSet" );
    settings.setValue( "version", HighlighterSet_VERSION );
    settings.setValue( "name", name_ );
    settings.setValue( "id", id_ );
    settings.remove( "highlighters" );
    settings.beginWriteArray( "highlighters" );
    for ( int i = 0; i < highlighterList_.size(); ++i ) {
        settings.setArrayIndex( i );
        highlighterList_[ i ].saveToStorage( settings );
    }
    settings.endArray();
    settings.endGroup();
}

void HighlighterSet::retrieveFromStorage( QSettings& settings )
{
    LOG_DEBUG << "HighlighterSet::retrieveFromStorage";

    highlighterList_.clear();

    if ( settings.contains( "FilterSet/version" ) ) {
        LOG_INFO << "HighlighterSet found old filters";
        settings.beginGroup( "FilterSet" );
        if ( settings.value( "version" ).toInt() <= FilterSet_VERSION ) {
            name_ = settings.value( "name", "Highlighters set" ).toString();
            id_ = settings.value( "id", generateIdFromUuid() ).toString();
            int size = settings.beginReadArray( "filters" );
            for ( int i = 0; i < size; ++i ) {
                settings.setArrayIndex( i );
                Highlighter highlighter;
                highlighter.retrieveFromStorage( settings );
                highlighterList_.append( std::move( highlighter ) );
            }
            settings.endArray();
        }
        else {
            LOG_ERROR << "Unknown version of filterSet, ignoring it...";
        }
        settings.endGroup();
        settings.remove( "FilterSet" );

        saveToStorage( settings );
        settings.sync();
    }
    else if ( settings.contains( "HighlighterSet/version" ) ) {
        settings.beginGroup( "HighlighterSet" );
        if ( settings.value( "version" ).toInt() <= HighlighterSet_VERSION ) {
            name_ = settings.value( "name", "Highlighters set" ).toString();
            id_ = settings.value( "id", generateIdFromUuid() ).toString();
            int size = settings.beginReadArray( "highlighters" );
            for ( int i = 0; i < size; ++i ) {
                settings.setArrayIndex( i );
                Highlighter highlighter;
                highlighter.retrieveFromStorage( settings );
                highlighterList_.append( std::move( highlighter ) );
            }
            settings.endArray();
        }
        else {
            LOG_ERROR << "Unknown version of highlighterSet, ignoring it...";
        }
        settings.endGroup();
    }
}

QList<HighlighterSet> HighlighterSetCollection::highlighterSets() const
{
    return highlighters_;
}

void HighlighterSetCollection::setHighlighterSets( const QList<HighlighterSet>& highlighters )
{
    highlighters_ = highlighters;
}

HighlighterSet HighlighterSetCollection::currentSet() const
{
    auto set = std::find_if( highlighters_.begin(), highlighters_.end(),
                             [ this ]( const auto& s ) { return s.id() == currentSet_; } );

    if ( set != highlighters_.end() ) {
        return *set;
    }
    else {
        return {};
    }
}

QString HighlighterSetCollection::currentSetId() const
{
    return currentSet_;
}

void HighlighterSetCollection::setCurrentSet( const QString& current )
{
    currentSet_ = current;
}

bool HighlighterSetCollection::hasSet( const QString& setId ) const
{
    return std::any_of( highlighters_.begin(), highlighters_.end(),
                        [ setId ]( const auto& s ) { return s.id() == setId; } );
}

void HighlighterSetCollection::saveToStorage( QSettings& settings ) const
{
    LOG_DEBUG << "HighlighterSetCollection::saveToStorage";

    settings.beginGroup( "HighlighterSetCollection" );
    settings.setValue( "version", HighlighterSetCollection_VERSION );
    settings.setValue( "current", currentSet_ );
    settings.remove( "sets" );
    settings.beginWriteArray( "sets" );
    for ( int i = 0; i < highlighters_.size(); ++i ) {
        settings.setArrayIndex( i );
        highlighters_[ i ].saveToStorage( settings );
    }
    settings.endArray();
    settings.endGroup();
}

void HighlighterSetCollection::retrieveFromStorage( QSettings& settings )
{
    LOG_DEBUG << "HighlighterSetCollection::retrieveFromStorage";

    highlighters_.clear();

    if ( settings.contains( "HighlighterSetCollection/version" ) ) {
        settings.beginGroup( "HighlighterSetCollection" );
        if ( settings.value( "version" ).toInt() <= HighlighterSetCollection_VERSION ) {
            int size = settings.beginReadArray( "sets" );
            for ( int i = 0; i < size; ++i ) {
                settings.setArrayIndex( i );
                HighlighterSet highlighterSet;
                highlighterSet.retrieveFromStorage( settings );
                highlighters_.append( std::move( highlighterSet ) );
            }
            settings.endArray();
            auto currentSet = settings.value( "current" ).toString();
            setCurrentSet( currentSet );
        }
        else {
            LOG_ERROR << "Unknown version of HighlighterSetCollection, ignoring it...";
        }
        settings.endGroup();
    }

    HighlighterSet oldHighlighterSet;
    oldHighlighterSet.retrieveFromStorage( settings );
    if ( !oldHighlighterSet.isEmpty() ) {
        LOG_INFO << "Importing old HighlighterSet";
        setCurrentSet( oldHighlighterSet.id() );
        highlighters_.append( std::move( oldHighlighterSet ) );
        settings.remove( "HighlighterSet" );
        saveToStorage( settings );
    }

    LOG_INFO << "Loaded " << highlighters_.size() << " highlighter sets";
}
