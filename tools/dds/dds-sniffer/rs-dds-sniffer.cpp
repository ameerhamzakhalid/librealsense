// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

#include "rs-dds-sniffer.h"

#include <thread>
#include <memory>

#include <librealsense2/dds/dds-participant.h>
#include <fastdds/rtps/common/Guid.h>

#include <tclap/CmdLine.h>
#include <tclap/ValueArg.h>
#include <tclap/SwitchArg.h>

using namespace TCLAP;

//FastDDS GUID_t - 4 MSB bytes host, 4 bytes process, 4 bytes participant, 4 bytes entity ID (reader/writer)
//For example:
//  Participant 1                 - 01.0f.be.05.f0.09.86.b6.01.00.00.00|0.0.1.c1
//  Writer under participant 1    - 01.0f.be.05.f0.09.86.b6.01.00.00.00|0.0.1.2
//  Participant 2 of same process - 01.0f.be.05.f0.09.86.b6.02.00.00.00|0.0.1.c1
//  Reader under participant 2    - 01.0f.be.05.f0.09.86.b6.02.00.00.00|0.0.1.7
//  Participant 3 other process   - 01.0f.be.05.88.50.ea.4a.01.00.00.00|0.0.1.c1
//Note same host for all, participant and entity IDs may be repeat for different processes
//To differentiate entities of different participant with same name we append process GUID values to the name
constexpr uint8_t GUID_PROCESS_LOCATION = 4;

int main( int argc, char ** argv ) try
{
    librealsense::dds::dds_domain_id domain = 0;
    uint32_t seconds = 0;

    CmdLine cmd( "librealsense rs-dds-sniffer tool", ' ' );
    SwitchArg snapshot_arg( "s", "snapshot", "run momentarily taking a snapshot of the domain" );
    SwitchArg machine_readable_arg( "m", "machine-readable", "output entities in a way more suitable for automatic parsing" );
    ValueArg< librealsense::dds::dds_domain_id > domain_arg( "d", "domain", "select domain ID to listen on", false, 0, "0-232" );
    cmd.add( snapshot_arg );
    cmd.add( machine_readable_arg );
    cmd.add( domain_arg );
    cmd.parse( argc, argv );

    if( snapshot_arg.isSet() )
    {
        seconds = 3;
    }

    if( domain_arg.isSet() )
    {
        domain = domain_arg.getValue();
        if( domain > 232 )
        {
            std::cerr << "Invalid domain value, enter a value in the range [0, 232]" << std::endl;
            return EXIT_FAILURE;
        }
    }

    dds_sniffer snif;
    if( snif.init( domain, snapshot_arg.isSet(), machine_readable_arg.isSet() ) )
    {
        snif.run( seconds );
    }
    else
    {
        std::cerr << "Initialization failure" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
catch( const TCLAP::ExitException & )
{
    std::cerr << "Undefined exception while parsing command line arguments" << std::endl;
    return EXIT_FAILURE;
}
catch( const TCLAP::ArgException & e )
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch( const std::exception & e )
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

dds_sniffer::dds_sniffer()
    : _participant()
{
}

dds_sniffer::~dds_sniffer()
{
}

bool dds_sniffer::init( librealsense::dds::dds_domain_id domain, bool snapshot, bool machine_readable )
{
    _print_discoveries = !snapshot;
    _print_by_topics = snapshot;
    _print_machine_readable = machine_readable;

    //Set callbacks before calling _participant.init(), or some events, specifically on_participant_added, might get lost
    _participant.on_writer_added( [this]( librealsense::dds::dds_guid guid, char const* topic_name )
    {
        on_writer_added( guid, topic_name );  
    } );
    _participant.on_writer_removed( [this]( librealsense::dds::dds_guid guid, char const* topic_name )
    {
        on_writer_removed( guid, topic_name );
    } );
    _participant.on_reader_added( [this]( librealsense::dds::dds_guid guid, char const* topic_name )
    { 
        on_reader_added( guid, topic_name );
    } );
    _participant.on_reader_removed( [this]( librealsense::dds::dds_guid guid, char const* topic_name )
    {
        on_reader_removed( guid, topic_name );
    } );
    _participant.on_participant_added( [this]( librealsense::dds::dds_guid guid, char const* participant_name )
    {
        on_participant_added( guid, participant_name );
    } );
    _participant.on_participant_removed( [this]( librealsense::dds::dds_guid guid, char const* participant_name )
    {
        on_participant_removed( guid, participant_name );
    } );

    _participant.init( domain, "rs-dds-sniffer" );

    if (!_print_machine_readable)
    {
        if (snapshot)
        {
            std::cout << "rs-dds-sniffer taking a snapshot of domain " << domain << std::endl;
        }
        else
        {
            std::cout << "rs-dds-sniffer listening on domain " << domain << std::endl;
        }
    }

    return _participant.is_valid();
}

void dds_sniffer::run( uint32_t seconds )
{
    if( seconds == 0 )
    {
        std::cin.ignore( std::numeric_limits< std::streamsize >::max() );
    }
    else
    {
        std::this_thread::sleep_for( std::chrono::seconds( seconds ) );
    }

    if( _print_by_topics )
    {
        if (_print_machine_readable)
        {
            print_topics_machine_readable();
        }
        else
        {
            print_topics();
        }
    }
}

void dds_sniffer::on_writer_added( librealsense::dds::dds_guid guid, const char* topic_name )
{
    if (_print_discoveries)
    {
        print_writer_discovered( guid, topic_name, true );
    }

    save_topic_writer( guid, topic_name );
}

void dds_sniffer::on_writer_removed( librealsense::dds::dds_guid guid, const char* topic_name )
{
    if( _print_discoveries )
    {
        print_writer_discovered( guid, topic_name, false );
    }

    remove_topic_writer( guid, topic_name );
}

void dds_sniffer::on_reader_added( librealsense::dds::dds_guid guid, const char* topic_name )
{
    if (_print_discoveries)
    {
        print_reader_discovered( guid, topic_name, true );
    }
    save_topic_reader( guid, topic_name );
}

void dds_sniffer::on_reader_removed( librealsense::dds::dds_guid guid, const char* topic_name )
{
    if (_print_discoveries)
    {
        print_reader_discovered( guid, topic_name, false );
    }

    remove_topic_reader( guid, topic_name );
}

void dds_sniffer::on_participant_added( librealsense::dds::dds_guid guid, const char* participant_name )
{
    if (_print_discoveries)
    {
        print_participant_discovered( guid, participant_name, true );
    }

    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    _discovered_participants[guid] = participant_name;
}
void dds_sniffer::on_participant_removed( librealsense::dds::dds_guid guid, const char* participant_name )
{
    if( _print_discoveries )
    {
        print_participant_discovered( guid, participant_name, false );
    }

    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    _discovered_participants.erase( guid );
}

void dds_sniffer::save_topic_writer( librealsense::dds::dds_guid guid, const char* topic_name )
{
    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    _discovered_topics[topic_name].writers.insert( guid );
}

void dds_sniffer::remove_topic_writer( librealsense::dds::dds_guid guid, const char* topic_name )
{
    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    auto topic_entry = _discovered_topics.find( topic_name );
    if(topic_entry != _discovered_topics.end() )
    {
        topic_entry->second.writers.erase( guid );
        if( topic_entry->second.writers.empty() && topic_entry->second.readers.empty() )
        {
            _discovered_topics.erase( topic_entry );
        }
    }
}

void dds_sniffer::save_topic_reader( librealsense::dds::dds_guid guid, const char* topic_name )
{
    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    _discovered_topics[topic_name].readers.insert( guid );
}

void dds_sniffer::remove_topic_reader( librealsense::dds::dds_guid guid, const char* topic_name )
{
    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    auto topic_entry = _discovered_topics.find( topic_name );
    if(topic_entry != _discovered_topics.end() )
    {
        topic_entry->second.readers.erase( guid );
        if( topic_entry->second.writers.empty() && topic_entry->second.readers.empty() )
        {
            _discovered_topics.erase(topic_entry);
        }
    }
}

uint32_t dds_sniffer::calc_max_indentation() const
{
    uint32_t indentation = 0;
    uint32_t max_indentation = 0;

    for (auto topic : _discovered_topics) //_dds_entities_lock locked by print_topics()
    {
        indentation = static_cast<uint32_t>(std::count( topic.first.begin(), topic.first.end(), '/' ));  // Use / as delimiter for nested topic names
        if (indentation >= max_indentation)
        {
            max_indentation = indentation + 1; //+1 for Reader/Writer indentation
        }
    }

    return max_indentation;
}

void dds_sniffer::print_writer_discovered( librealsense::dds::dds_guid guid, const char* topic_name, bool discovered ) const
{
    if (_print_machine_readable)
    {
        std::cout << "DataWriter," << guid << "," << topic_name << (discovered ? ",discovered" : ",removed") << std::endl;
    }
    else
    {
        std::cout << "DataWriter  " << guid << " publishing topic '" << topic_name 
                  << (discovered ? "' discovered" : "' removed") << std::endl;
    }
}

void dds_sniffer::print_reader_discovered( librealsense::dds::dds_guid guid, const char* topic_name, bool discovered ) const
{
    if (_print_machine_readable)
    {
        std::cout << "DataReader," << guid << "," << topic_name << (discovered ? ",discovered" : ",removed") << std::endl;
    }
    else
    {
        std::cout << "DataReader  " << guid << " reading topic '" << topic_name
                  << (discovered ? "' discovered" : "' removed") << std::endl;
    }
}

void dds_sniffer::print_participant_discovered( librealsense::dds::dds_guid guid, const char* participant_name, bool discovered ) const
{
    if (_print_machine_readable)
    {
        std::cout << "Participant," << guid << "," << participant_name
                  << (discovered ? ",discovered" : ",removed") << std::endl;
    }
    else
    {
        uint16_t tmp( 0 );
        memcpy( &tmp, &guid.guidPrefix.value[GUID_PROCESS_LOCATION], sizeof( tmp ) );
        std::cout << "Participant " << guid << " " << participant_name << "_" << std::hex << tmp
                  << std::dec << (discovered ? " discovered" : " removed") << std::endl;
    }
}

void dds_sniffer::print_topics_machine_readable() const
{
    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    for (auto topic : _discovered_topics)
    {
        for (auto writer : topic.second.writers)
        {
            std::cout << topic.first << ",";
            print_topic_writer( writer );
        }
        for (auto reader : topic.second.readers)
        {
            std::cout << topic.first << ",";
            print_topic_reader( reader );
        }
    }
}

void dds_sniffer::print_topics() const
{
    std::istringstream last_topic( "" );
    std::string last_topic_nested;

    std::lock_guard<std::mutex> lock( _dds_entities_lock );

    uint32_t max_indentation( calc_max_indentation() );

    for( auto topic : _discovered_topics )
    {
        std::cout << std::endl;

        std::istringstream current_topic( topic.first );  // Get topic name
        std::string current_topic_nested;
        uint32_t indentation = 0;

        // Compare to previous topic
        while( std::getline( last_topic, last_topic_nested, '/' ) )  // Use / as delimiter for nested topic names
        {
            if( std::getline( current_topic, current_topic_nested, '/' ) )
            {
                if( current_topic_nested.compare( last_topic_nested ) == 0 )
                {
                    ++indentation;  // Skip parts that are same as previous topic
                }
                else
                {
                    ident( indentation );
                    std::cout << current_topic_nested << std::endl;
                    ++indentation;
                    break;
                }
            }
        }

        // Print reminder of string
        while( std::getline( current_topic, current_topic_nested, '/' ) )
        {
            ident( indentation );
            std::cout << current_topic_nested << std::endl;
            ++indentation;
        }

        for( auto writer : topic.second.writers )
        {
            print_topic_writer( writer, max_indentation );
        }
        for( auto reader : topic.second.readers )
        {
            print_topic_reader( reader, max_indentation );
        }

        last_topic.clear();
        last_topic.str( topic.first );  // Save topic name for next iteration
        last_topic.seekg( 0, last_topic.beg );
    }
}

void dds_sniffer::ident( uint32_t indentation ) const
{
    while( indentation > 0 )
    {
        std::cout << "    ";
        --indentation;
    }
    std::cout << "- ";
}

void dds_sniffer::print_topic_writer( librealsense::dds::dds_guid guid, uint32_t indentation ) const
{
    auto iter = _discovered_participants.begin();
    for( ; iter != _discovered_participants.end(); ++iter ) //_dds_entities_lock locked by caller
    {
        if( iter->first.guidPrefix == guid.guidPrefix )
        {
            uint16_t tmp;
            memcpy( &tmp, &iter->first.guidPrefix.value[GUID_PROCESS_LOCATION], sizeof( tmp ) );
            if (_print_machine_readable)
            {
                std::cout << "Writer," << iter->second << "_" << std::hex << std::setw(4) << std::setfill( '0' ) << tmp << std::dec << std::endl;
            }
            else
            {
                ident( indentation );
                std::cout << "Writer of \"" << iter->second << "_" << std::hex << std::setw(4) << std::setfill( '0' ) << tmp << std::dec << "\"" << std::endl;
            }
            break;
        }
    }
    if( iter == _discovered_participants.end() )
    {
        ident( indentation );
        std::cout << "Writer of unknown participant" << std::endl;
    }
}

void dds_sniffer::print_topic_reader( librealsense::dds::dds_guid guid, uint32_t indentation ) const
{
    auto iter = _discovered_participants.begin();
    for( ; iter != _discovered_participants.end(); ++iter ) //_dds_entities_lock locked by caller
    {
        if( iter->first.guidPrefix == guid.guidPrefix )
        {
            uint16_t tmp;
            memcpy( &tmp, &iter->first.guidPrefix.value[GUID_PROCESS_LOCATION], sizeof( tmp ) );
            if (_print_machine_readable)
            {
                std::cout << "Reader," << iter->second << "_" << std::hex << std::setw(4) << std::setfill( '0' ) << tmp << std::dec << std::endl;
            }
            else
            {
                ident( indentation );
                std::cout << "Reader of \"" << iter->second << "_" << std::hex << std::setw(4) << std::setfill( '0' ) << tmp << std::dec << "\"" << std::endl;
            }
            break;
        }
    }
    if( iter == _discovered_participants.end() )
    {
        ident( indentation );
        std::cout << "Reader of unknown participant" << std::endl;
    }
}