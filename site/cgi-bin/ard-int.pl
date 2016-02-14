#!/opt/local/bin/perl
################################################################################
#
#   API PATHS
#       /           everything (TODO currently only GET supported)
#       /clock      clock time only
#       /alarms     all alarms
#       /alarms/0   alarm 0
#       /alarms/1   alarm 1
#       /alarms/2   alarm 2
#       /alarms/3   alarm 3
#
#   POST and RESPONSE DATA
#       JSON formatted data
#       { "clock":{"hr":h,"mn":m,"sc":s,"dy",d } }
#       { "clock":"local" } -> sets clock to server local time
#       { "alarms":[{"hr":h,"mn":,"sc":s,"dy":d},
#                   {"hr":h,"mn":,"sc":s,"dy":d},
#                   {"hr":h,"mn":,"sc":s,"dy":d},
#                   {"hr":h,"mn":,"sc":s,"dy":d}] }
#       { "alarm":{"num":n,"hr":h,"mn":,"sc":s,"dy":d} }
#
################################################################################

use LWP::UserAgent;
use CGI;
use JSON;

use Log::Log4perl qw( get_logger :levels);

use ArduinoInterface;
use ArduinoClock;
use ArduinoAlarms;
use ArduinoAlarm;

use warnings;
use strict;

# Logging Setup
Log::Log4perl->init_and_watch( "perl_logger.conf", 60 );
my $local_logger = get_logger( "ard_int" );



####################################################################
# TODO need to find a way to not use fixed IP address here!!!!
####################################################################
my $ard_ip = "192.168.0.181";
#my $ard_ip = "Arduino.local";
my $ard = ArduinoInterface->new( IP_ADDR => $ard_ip );


my $cgi_query = CGI->new;

$local_logger->debug( "Starting CGI request processing.");


foreach (sort keys %ENV) {
    $local_logger->debug( "$_: $ENV{$_}" );
}

# PATH_INFO environment variable contains the appropriate part of the
# request URI path for the API
my $req_item = $cgi_query->path_info();
$local_logger->info( "CGI PATH_INFO: $req_item" );

my $resp_str;
my $post = 0;
my $post_data;
my $post_str;
if ( "GET" eq $cgi_query->request_method() ) {
    $local_logger->info( "CGI GET request received." );
    $post = 0;
} elsif ( "POST" eq $cgi_query->request_method() ) {
    $local_logger->info( "CGI POST request received." );
    $post = 1;
    if ( $post_str = $cgi_query->param( 'POSTDATA' ) ) {
        $post_data = decode_json $post_str;
    } else {
        # POSTDATA is not valid JSON data
        $local_logger->info( "CGI POST data is not valid JSON data." );
    }
} else {
    # Only GET and POST supported
    $local_logger->info( "CGI request: not a GET or POST." );
    SendBadRequest();
    exit;
}

$local_logger->debug( "CGI request: parsing request item." );
# split is returns an empty string as first element because of leading /
my @req = split( '/', $req_item );
if ( defined( $_ = shift @req ) ) {
    unless ( length ) {
        # Empty element for leading /
        if ( defined( $_ = shift @req ) ) {
            if ( /^clock$/ ) {
                $local_logger->debug( "CGI request: clock." );
                my $tm;
                if ( $post ) {
                    $local_logger->debug( "CGI request: setting clock." );
                    if ( "localtime" eq $post_data->{'clock'} ) {
                        $local_logger->debug( "CGI request: setting clock to localtime." );
                        $tm = $ard->SetArduinoTime;
                    } else {
                        my $cl = ArduinoClock->new;
                        unless( defined( $cl->set_ua_json_str( $post_str ) ) ) {
                            $local_logger->debug("POST clock: set_ua_json_str failed for clock");
                            SendBadRequest();
                            exit;
                        }

                        $tm = $ard->SetArduinoTime( $cl );
                    }
                } else {
                    $tm = $ard->GetArduinoTime;
                }
                if ( $tm ) {
                    $local_logger->debug( "CGI request: building clock response." );
                    $resp_str = $tm->get_ua_json_str;
                } else {
                    $local_logger->info( "CGI request: failed to fetch clock data from Arduino." );
                    SendServerError();
                    exit;
                }
            } elsif ( /^alarms$/ ) {
                $local_logger->debug( "CGI request: alarms." );
                my $al = ArduinoAlarms->new;
                my $alarm;
                if ( defined( $_ = shift @req ) ) {
                    # Single Alarm
                    $local_logger->debug( "CGI request: alarms - single." );
                    $alarm = $al->get_alarm( $_ );
                } else {
                    # All Alarms
                    $local_logger->debug( "CGI request: alarms - all." );
                    $alarm = ArduinoAlarms->new;
                }

                if ( $post ) {
                    # This will send bad respons if any values are missing in
                    # post_data, or out of range
                    unless( defined( $alarm->set_ua_json_str( $post_str ) ) ) {
                        $local_logger->debug("POST alarms: set_ua_json_str failed for alarm.");
                        SendBadRequest();
                        exit;
                    }
                    $al = $ard->SetArduinoAlarm( $alarm );
                } else {
                    # This should work for no alarms, $_ is undefined, same
                    # as no argument
                    $al = $ard->GetArduinoAlarm( $_ );
                }
                if ( $al ) {
                    $local_logger->debug( "CGI request: building alarm response." );
                    $resp_str = $al->get_ua_json_str;
                } else {
                    $local_logger->info( "CGI request: failed to fetch alarm data from Arduino." );
                    SendServerError();
                    exit;
                }
            } else {
                $local_logger->info( "CGI request: bad request URI." );
                SendBadRequest();
                exit;
            }
        } else {
            # TODO what's this??? thought it was / only, but it's not?
            # Is it even possible???
            $local_logger->info( "CGI request: confused?." );
            SendBadRequest();
            exit;
        }
    } else {
        # TODO can this ever happen?
        # non-zero length first element
        $local_logger->info( "CGI request: unexpected URI format." );
        SendBadRequest();
        exit;
    }
} else {
    # No elements after splitting path_info
    # indicates 'root', get everything
    $local_logger->info( "CGI request: everything." );

    my $resp_data = {};
    my $resp;
    if ( $post ) {
        $local_logger->info( "CGI POST request: everything not supported." );
        ##############################################################
        ##############################################################
        ##############################################################
        # TODO post everything not implemented, should it be?
        # TODO UPDATE should be easy to add this now, need a set_ua_json_str
        # method in ArduinoInterfac.pm
        # TODO this should really be a "405 Method Not Allowed" status
        ##############################################################
        ##############################################################
        ##############################################################
        SendBadRequest();
        exit;
    } else {
        $resp = $ard->GetArduinoAll;
    }
    if ( $resp ) {
        $resp_str = $resp->get_ua_json_str;
    } else {
        $local_logger->info( "CGI request: failed to fetch clock data from Arduino." );
        SendServerError();
        exit;
    }
}




if ( $resp_str ) { 
    print $cgi_query->header( -type => 'application/json' );
    print $resp_str; 
}



sub SendBadRequest
{
    print $cgi_query->header( -type => 'text/html',
                              -status=>'400 Bad Request' );
    # TODO maybe add some useful information to response??
}

sub SendServerError
{
    print $cgi_query->header( -type => 'text/html',
                              -type => '500 Internal Server Error' );
    # TODO maybe add some useful information to response??
}




