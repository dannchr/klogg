/*
 * Copyright (C) 2009, 2010, 2013, 2015 Nicolas Bonnefon and other contributors
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
 * Copyright (C) 2016 -- 2019 Anton Filimonov and other contributors
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

#include <mutex>

#include <QFontInfo>

#include "configuration.h"
#include "log.h"
#include "styles.h"

namespace {
std::once_flag fontInitFlag;
static const Configuration DefaultConfiguration = {};
} // namespace

Configuration::Configuration()
{
    splitterSizes_ << 400 << 100;
}

// Accessor functions
QFont Configuration::mainFont() const
{
    std::call_once( fontInitFlag, [ this ]() {
        mainFont_.setStyleHint( QFont::Courier, QFont::PreferOutline );

        QFontInfo fi( mainFont_ );
        LOG_INFO << "DefaultConfiguration font is " << fi.family().toStdString() << ": "
                 << fi.pointSize();
    } );

    return mainFont_;
}

void Configuration::setMainFont( QFont newFont )
{
    LOG_DEBUG << "Configuration::setMainFont";

    mainFont_ = newFont;
}

void Configuration::retrieveFromStorage( QSettings& settings )
{
    LOG_DEBUG << "Configuration::retrieveFromStorage";

    // Fonts
    QString family
        = settings.value( "mainFont.family", DefaultConfiguration.mainFont_.family() ).toString();
    int size
        = settings.value( "mainFont.size", DefaultConfiguration.mainFont_.pointSize() ).toInt();

    // If no config read, keep the DefaultConfiguration
    if ( !family.isNull() )
        mainFont_ = QFont( family, size );

    forceFontAntialiasing_
        = settings.value( "mainFont.antialiasing", DefaultConfiguration.forceFontAntialiasing_ )
              .toBool();

    enableQtHighDpi_
        = settings.value( "view.qtHiDpi", DefaultConfiguration.enableQtHighDpi_ ).toBool();

    scaleFactorRounding_
        = settings.value( "view.scaleFactorRounding", DefaultConfiguration.scaleFactorRounding_ )
              .toInt();

    // Regexp types
    mainRegexpType_ = static_cast<SearchRegexpType>(
        settings
            .value( "regexpType.main", static_cast<int>( DefaultConfiguration.mainRegexpType_ ) )
            .toInt() );
    quickfindRegexpType_ = static_cast<SearchRegexpType>(
        settings
            .value( "regexpType.quickfind",
                    static_cast<int>( DefaultConfiguration.quickfindRegexpType_ ) )
            .toInt() );
    regexpEngine_ = static_cast<RegexpEngine>(
        settings
            .value( "regexpType.engine", static_cast<int>( DefaultConfiguration.regexpEngine_ ) )
            .toInt() );
    quickfindIncremental_
        = settings.value( "quickfind.incremental", DefaultConfiguration.quickfindIncremental_ )
              .toBool();

    // "Advanced" settings
    nativeFileWatchEnabled_
        = settings.value( "nativeFileWatch.enabled", DefaultConfiguration.nativeFileWatchEnabled_ )
              .toBool();
    settings.remove( "nativeFileWatch.enabled" );
    nativeFileWatchEnabled_
        = settings.value( "filewatch.useNative", nativeFileWatchEnabled_ ).toBool();

    pollingEnabled_
        = settings.value( "polling.enabled", DefaultConfiguration.pollingEnabled_ ).toBool();
    settings.remove( "polling.enabled" );
    pollingEnabled_ = settings.value( "filewatch.usePolling", pollingEnabled_ ).toBool();

    pollIntervalMs_
        = settings.value( "polling.intervalMs", DefaultConfiguration.pollIntervalMs_ ).toInt();
    settings.remove( "polling.intervalMs" );
    pollIntervalMs_ = settings.value( "filewatch.pollingIntervalMs", pollIntervalMs_ ).toInt();

    fastModificationDetection_ = settings
                                     .value( "filewatch.fastModificationDetection",
                                             DefaultConfiguration.fastModificationDetection_ )
                                     .toBool();

    loadLastSession_
        = settings.value( "session.loadLast", DefaultConfiguration.loadLastSession_ ).toBool();
    allowMultipleWindows_
        = settings.value( "session.multipleWindows", DefaultConfiguration.allowMultipleWindows_ )
              .toBool();
    followFileOnLoad_
        = settings.value( "session.followOnLoad", DefaultConfiguration.followFileOnLoad_ ).toBool();

    enableLogging_
        = settings.value( "logging.enableLogging", DefaultConfiguration.enableLogging_ ).toBool();
    loggingLevel_ = static_cast<uint8_t>(
        settings.value( "logging.verbosity", DefaultConfiguration.loggingLevel_ ).toInt() );

    enableVersionChecking_
        = settings.value( "versionchecker.enabled", DefaultConfiguration.enableVersionChecking_ )
              .toBool();

    extractArchives_
        = settings.value( "archives.extract", DefaultConfiguration.extractArchives_ ).toBool();
    extractArchivesAlways_
        = settings.value( "archives.extractAlways", DefaultConfiguration.extractArchivesAlways_ )
              .toBool();

    // "Perf" settings
    useParallelSearch_
        = settings.value( "perf.useParallelSearch", DefaultConfiguration.useParallelSearch_ )
              .toBool();
    useSearchResultsCache_
        = settings
              .value( "perf.useSearchResultsCache", DefaultConfiguration.useSearchResultsCache_ )
              .toBool();
    searchResultsCacheLines_ = settings
                                   .value( "perf.searchResultsCacheLines",
                                           DefaultConfiguration.searchResultsCacheLines_ )
                                   .toInt();
    indexReadBufferSizeMb_
        = settings
              .value( "perf.indexReadBufferSizeMb", DefaultConfiguration.indexReadBufferSizeMb_ )
              .toInt();
    searchReadBufferSizeLines_ = settings
                                     .value( "perf.searchReadBufferSizeLines",
                                             DefaultConfiguration.searchReadBufferSizeLines_ )
                                     .toInt();
    searchThreadPoolSize_
        = settings.value( "perf.searchThreadPoolSize", DefaultConfiguration.searchThreadPoolSize_ )
              .toInt();
    keepFileClosed_
        = settings.value( "perf.keepFileClosed", DefaultConfiguration.keepFileClosed_ ).toBool();
    useLineEndingCache_
        = settings.value( "perf.useLineEndingCache", DefaultConfiguration.useLineEndingCache_ )
              .toBool();

    verifySslPeers_
        = settings.value( "net.verifySslPeers", DefaultConfiguration.verifySslPeers_ ).toBool();

    // View settings
    overviewVisible_
        = settings.value( "view.overviewVisible", DefaultConfiguration.overviewVisible_ ).toBool();
    lineNumbersVisibleInMain_ = settings
                                    .value( "view.lineNumbersVisibleInMain",
                                            DefaultConfiguration.lineNumbersVisibleInMain_ )
                                    .toBool();
    lineNumbersVisibleInFiltered_ = settings
                                        .value( "view.lineNumbersVisibleInFiltered",
                                                DefaultConfiguration.lineNumbersVisibleInFiltered_ )
                                        .toBool();
    minimizeToTray_
        = settings.value( "view.minimizeToTray", DefaultConfiguration.minimizeToTray_ ).toBool();

    style_ = settings.value( "view.style", DefaultConfiguration.style_ ).toString();

    auto styles = availableStyles();
    if ( !styles.contains( style_ ) ) {
        style_ = defaultPlatformStyle();
    }
    if ( !styles.contains( style_ ) ) {
        style_ = styles.front();
    }

    // Some sanity check (mainly for people upgrading)
    if ( quickfindIncremental_ )
        quickfindRegexpType_ = SearchRegexpType::FixedString;

    // DefaultConfiguration crawler settings
    searchAutoRefresh_ = settings
                             .value( "DefaultConfigurationView.searchAutoRefresh",
                                     DefaultConfiguration.searchAutoRefresh_ )
                             .toBool();
    searchIgnoreCase_ = settings
                            .value( "DefaultConfigurationView.searchIgnoreCase",
                                    DefaultConfiguration.searchIgnoreCase_ )
                            .toBool();

    if ( settings.contains( "DefaultConfigurationView.splitterSizes" ) ) {
        splitterSizes_.clear();

        const auto sizes = settings.value( "DefaultConfigurationView.splitterSizes" ).toList();
        std::transform( sizes.begin(), sizes.end(), std::back_inserter( splitterSizes_ ),
                        []( auto v ) { return v.toInt(); } );
    }
}

void Configuration::saveToStorage( QSettings& settings ) const
{
    LOG_DEBUG << "Configuration::saveToStorage";

    QFontInfo fi( mainFont_ );

    settings.setValue( "mainFont.family", fi.family() );
    settings.setValue( "mainFont.size", fi.pointSize() );
    settings.setValue( "mainFont.antialiasing", forceFontAntialiasing_ );
    settings.setValue( "regexpType.main", static_cast<int>( mainRegexpType_ ) );
    settings.setValue( "regexpType.quickfind", static_cast<int>( quickfindRegexpType_ ) );
    settings.setValue( "regexpType.engine", static_cast<int>( regexpEngine_ ) );
    settings.setValue( "quickfind.incremental", quickfindIncremental_ );
    settings.setValue( "filewatch.useNative", nativeFileWatchEnabled_ );
    settings.setValue( "filewatch.usePolling", pollingEnabled_ );
    settings.setValue( "filewatch.pollingIntervalMs", pollIntervalMs_ );
    settings.setValue( "filewatch.fastModificationDetection", fastModificationDetection_ );
    settings.setValue( "session.loadLast", loadLastSession_ );
    settings.setValue( "session.multipleWindows", allowMultipleWindows_ );
    settings.setValue( "session.followOnLoad", followFileOnLoad_ );

    settings.setValue( "logging.enableLogging", enableLogging_ );
    settings.setValue( "logging.verbosity", loggingLevel_ );

    settings.setValue( "versionchecker.enabled", enableVersionChecking_ );

    settings.setValue( "archives.extract", extractArchives_ );
    settings.setValue( "archives.extractAlways", extractArchivesAlways_ );

    settings.setValue( "perf.useParallelSearch", useParallelSearch_ );
    settings.setValue( "perf.useSearchResultsCache", useSearchResultsCache_ );
    settings.setValue( "perf.searchResultsCacheLines", searchResultsCacheLines_ );
    settings.setValue( "perf.indexReadBufferSizeMb", indexReadBufferSizeMb_ );
    settings.setValue( "perf.searchReadBufferSizeLines", searchReadBufferSizeLines_ );
    settings.setValue( "perf.searchThreadPoolSize", searchThreadPoolSize_ );
    settings.setValue( "perf.keepFileClosed", keepFileClosed_ );
    settings.setValue( "perf.useLineEndingCache", useLineEndingCache_ );

    settings.setValue( "net.verifySslPeers", verifySslPeers_ );

    settings.setValue( "view.overviewVisible", overviewVisible_ );
    settings.setValue( "view.lineNumbersVisibleInMain", lineNumbersVisibleInMain_ );
    settings.setValue( "view.lineNumbersVisibleInFiltered", lineNumbersVisibleInFiltered_ );
    settings.setValue( "view.minimizeToTray", minimizeToTray_ );
    settings.setValue( "view.style", style_ );

    settings.setValue( "view.qtHiDpi", enableQtHighDpi_ );
    settings.setValue( "view.scaleFactorRounding", scaleFactorRounding_ );

    settings.setValue( "DefaultConfigurationView.searchAutoRefresh", searchAutoRefresh_ );
    settings.setValue( "DefaultConfigurationView.searchIgnoreCase", searchIgnoreCase_ );

    QList<QVariant> splitterSizes;
    std::transform( splitterSizes_.begin(), splitterSizes_.end(),
                    std::back_inserter( splitterSizes ),
                    []( auto s ) { return QVariant::fromValue( s ); } );

    settings.setValue( "DefaultConfigurationView.splitterSizes", splitterSizes );
}
