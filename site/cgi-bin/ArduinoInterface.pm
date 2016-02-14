
package ArduinoInterface;
require Exporter;
use LWP::UserAgent;
use JSON;
use Log::Log4perl qw( get_logger :levels);
use Data::Dumper;
use ArduinoClock;
use ArduinoAlarms;
use ArduinoAlarm;

use strict;

our $VERSION = 1.00;
our @ISA = qw(Exporter);
our @EXPORT = ();
our @EXPORT_OK = qw( );
#our @EXPORT_OK = qw( EncodeArduinoAlarm EncodeArduinoClock DecodeArduinoAlarm DecodeArduinoClock GetArduinoEncodedLocaltime);
our @EXPORT_TAGS = ();

# Object Instance Variables
#   IP_ADDR - IP address of the Arduino
#   CLOCK - ArduinoClock object
#   ALARMS - ArduinoAlarms object 
#   OBJ_LOGGER - Log4perl object


# Object constructor
sub new
{
    my $invocant = shift; # first argument for class method is the invocant (class name or obj reference)
    my $class = ref($invocant) || $invocant;
    my $object_logger = get_logger( "ArduinoInterface" );
    my $self = {
        IP_ADDR => "",
        CLOCK => ArduinoClock->new,
        ALARMS => ArduinoAlarms->new,
        OBJ_LOGGER => $object_logger,
        @_, # remaining arguments will override default values
    };
    bless( $self, $class );
    return $self;
}

sub get_ua_json_str
{
    my $self = shift;

    return encode_json( { %{decode_json( $self->{'CLOCK'}->get_ua_json_str )},
                          %{decode_json( $self->{'ALARMS'}->get_ua_json_str )} } );
}

sub GetArduinoAll
{
    # No parameters expected
    # Updates values of objects ArduinoClock and ArduinoAlarms member
    # Returns ArduinoInterface object or undef on error
    my $self = shift;

    my $local_logger = $self->{OBJ_LOGGER};

    my $ua = LWP::UserAgent->new;
    my $server_endpoint = "http://" . $self->{IP_ADDR} . "/";
    my $req;
    $req = HTTP::Request->new( GET => $server_endpoint );
    $local_logger->info( "HTTP GET / sending request..." );
    my $resp = $ua->request($req);

    if ( $resp->is_success ) {
        $local_logger->info( "HTTP GET / response received." );

        if ( !defined( $self->{'CLOCK'}->set_ar_json_str( $resp->decoded_content ) ) ) {
            $local_logger->error( "Failed to update clock object with HTTP response data." );
            $local_logger->info( "HTTP response: " . $resp->decoded_content );
            return undef;
        }

        if ( !defined( $self->{'ALARMS'}->set_ar_json_str( $resp->decoded_content ) ) ) {
            $local_logger->error( "Failed to update alarms object with HTTP response data." );
            $local_logger->debug( "HTTP response: " . $resp->decoded_content );
            return undef;
        }

        return $self;
    } else {
        $local_logger->error( "HTTP GET / error code: ", $resp->code );
        $local_logger->error( "HTTP GET / error message: ", $resp->message );
        return undef;
    }
}

sub SetArduinoAll
{
    # Arguments:
    #   JSON string for everything
    #
    # Updates values of object ArduinoClock member to response value
    # Returns ArduinoClock object or undef on error
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};


    my $ua = LWP::UserAgent->new;
    my $server_endpoint = "http://" . $self->{IP_ADDR} . "/";
    my $req;

    $req = HTTP::Request->new( POST => $server_endpoint );
    $local_logger->info( "HTTP POST / sending request..." );

    my $input_json = shift;
    $local_logger->debug( "Input String: $input_json" );
    $self->{'CLOCK'}->set_ua_json_str( $input_json );
    $self->{'ALARMS'}->set_ua_json_str( $input_json );

    #############################################################################################
    #############################################################################################
    #############################################################################################
    # TODO this works correctly, but Arduino doesn't like it because LED1 is
    # missing. Need to add LED1 to interface!!!
    #############################################################################################
    #############################################################################################
    #############################################################################################
    my $req_content = encode_json( { %{decode_json( $self->{'CLOCK'}->get_ar_json_str )},
                                     %{decode_json( $self->{'ALARMS'}->get_ar_json_str )} } );
    $local_logger->debug( "Output String: $req_content" );

    $req->content( $req_content );

    my $resp = $ua->request($req);

    if ( $resp->is_success ) {
        $local_logger->info( "HTTP GET / response received." );

        if ( !defined( $self->{'CLOCK'}->set_ar_json_str( $resp->decoded_content ) ) ) {
            $local_logger->error( "Failed to update clock object with HTTP response data." );
            $local_logger->debug( "HTTP response: " . $resp->decoded_content );
            return undef;
        }

        if ( !defined( $self->{'ALARMS'}->set_ar_json_str( $resp->decoded_content ) ) ) {
            $local_logger->error( "Failed to update alarms object with HTTP response data." );
            $local_logger->debug( "HTTP response: " . $resp->decoded_content );
            return undef;
        }

        return $self;
    } else {
        $local_logger->error( "HTTP POST / error code: ", $resp->code );
        $local_logger->error( "HTTP POST / error message: ", $resp->message );
        return undef;
    }
}

sub GetArduinoTime
{
    # No parameters expected
    # Updates values of object ArduinoClock member
    # Returns ArduinoClock object or undef on error
    my $self = shift;

    my $local_logger = $self->{OBJ_LOGGER};

    my $ua = LWP::UserAgent->new;
    my $server_endpoint = "http://" . $self->{IP_ADDR} . "/Clock";
    my $req;
    $req = HTTP::Request->new( GET => $server_endpoint );
    $local_logger->info( "HTTP GET /Clock sending request..." );
    my $resp = $ua->request($req);

    if ( $resp->is_success ) {
        $local_logger->info( "HTTP GET /Clock response received." );

        if ( !defined( $self->{CLOCK}->set_ar_json_str( $resp->decoded_content ) ) ) {
            $local_logger->error( "Failed to update clock object with HTTP response data." );
            $local_logger->info( "HTTP response: " . $resp->decoded_content );
            return undef;
        }
        $local_logger->debug( "HTTP GET /Clock response data:" );
        $local_logger->debug( Dumper( $self->{CLOCK} ) );

        return $self->{CLOCK};
    } else {
        $local_logger->error( "HTTP GET /Clock error code: ", $resp->code );
        $local_logger->error( "HTTP GET /Clock error message: ", $resp->message );
        return undef;
    }
}

sub SetArduinoTime
{
    # Arguments:
    #   (none) - set to localtime
    #   ArduionClock object reference  - sets the with values in object
    #   LIST ( <hr>, <mn>, <sc>, <dy> )
    #
    # Updates values of object ArduinoClock member to response value
    # Returns ArduinoClock object or undef on error
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    my $clock;
    if ( $#_ == 0 ) {
        # only one argument, assume it is an object reference
        if (  "ArduinoClock" eq ref( $_[0] ) ) {
           $clock = $_[0];
        } else {
            # invalid arguments
            $local_logger->info( "Invalid argument, should be object reference to ArduinoClock" );
            return undef;
        }
    } else {
        $clock = ArduinoClock->new;

        if ( $#_ == 3 ) {
            # set clock with argument values
            unless( defined( $clock->hour( shift ) ) ) {
                $local_logger->debug( "Invalid clock hour in arg list." );
                return undef;
            }
            unless( defined( $clock->minute( shift ) ) ) {
                $local_logger->debug( "Invalid clock minute in arg list." );
                return undef;
            }
            unless( defined( $clock->second( shift ) ) ) {
                $local_logger->debug( "Invalid clock second in arg list." );
                return undef;
            }
            unless( defined( $clock->day( shift ) ) ) {
                $local_logger->debug( "Invalid clock day in arg list." );
                return undef;
            }
            $local_logger->info( "Set clock to custom time." );
        } elsif ( $#_ == -1 ) {
            # set clock to local time
            $local_logger->info( "Set clock to local time." );
            $clock->set_to_localtime;
        } else {
            $local_logger->info( "Invalid arguments to set clock." );
            return undef;
        }
    }

    my $ua = LWP::UserAgent->new;
    my $server_endpoint = "http://" . $self->{IP_ADDR} . "/Clock";
    my $req;

    $req = HTTP::Request->new( POST => $server_endpoint );
    $local_logger->info( "HTTP POST /Clock sending request..." );

    $req->content( $clock->get_ar_json_str );

    my $resp = $ua->request($req);

    if ( $resp->is_success ) {
        $local_logger->info( "HTTP GET /Clock response received." );

        if ( !defined( $self->{CLOCK}->set_ar_json_str( $resp->decoded_content ) ) ) {
            $local_logger->error( "Failed to update clock object with HTTP response data." );
            $local_logger->debug( "HTTP response: " . $resp->decoded_content );
            return undef;
        }
        $local_logger->debug( "HTTP POST /Clock response data:" );
        $local_logger->debug( Dumper( $self->{CLOCK} ) );

        return $self->{CLOCK};
    } else {
        $local_logger->error( "HTTP POST /Clock error code: ", $resp->code );
        $local_logger->error( "HTTP POST /Clock error message: ", $resp->message );
        return undef;
    }
}

sub GetArduinoAlarm
{
    # Parameters:
    #   (none) get all alarms
    #   0-3 - get the specified alarm - this is a single number from 0 to 3

    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    my $alarm_num = shift;

    my $req_uri = "/Alarms";
    if ( defined($alarm_num) ) {
        # Get the requested alarm
        if ( $alarm_num > 3 ) {
            # invalid index
            $local_logger->error( "Invalid alarm index requested" );
            return undef;
        } else {
            $req_uri .= "/";
            $req_uri .= ("A0", "A1", "A2", "A3")[$alarm_num];
        }
    } else {
        # Get all alarms
        # Nothing to do here
        $local_logger->info( "HTTP Get $req_uri" );
    }

    my $ua = LWP::UserAgent->new;
    my $server_endpoint = "http://" . $self->{IP_ADDR} . ${req_uri};
    my $req;
    $req = HTTP::Request->new( GET => $server_endpoint );
    # dd( $req );
    $local_logger->info( "HTTP GET $req_uri sending request..." );
    my $resp = $ua->request($req);

    # dd( $resp );

    if ( $resp->is_success ) {
        $local_logger->info( "HTTP GET $req_uri response received." );

        my $ret = $self->{ALARMS}->set_ar_json_str( $resp->decoded_content );

        if ( !defined( $ret ) ) {
            $local_logger->error( "Failed to update alarms object with HTTP response data." );
            $local_logger->debug( "HTTP response: " . $resp->decoded_content );
            return undef;
        }
        $local_logger->debug( "HTTP GET $req_uri response data:" );
        $local_logger->debug( Dumper( $self->{CLOCK} ) );

        return $ret;
    } else {
        $local_logger->error( "HTTP GET error code: ", $resp->code );
        $local_logger->error( "HTTP GET error message: ", $resp->message );
        return undef;
    }
}

sub SetArduinoAlarm
{
    # Arguments:
    #   ArduinoAlarms object reference - sets all alarms based on values in object
    #   Arduionalarm object reference  - sets the alarm with name in object to
    #           object values
    #   LIST ( <num>, <hr>, <mn>, <sc>, <dy> )
    #       
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    $local_logger->debug( "SetArduinoAlarm arguments: @_" );
    my $alarm;
    if ( 0 == $#_ ) {
        # only one argument, assume it is an object reference
        if (  ( "ArduinoAlarms" eq ref( $_[0] ) )
           || ( "ArduinoAlarm" eq ref( $_[0] ) ) ) {
           $alarm = $_[0];
        } else {
            # invalid arguments
            $local_logger->info( "Invalid argument, should be object reference to ArduinoAlarm(s)" );
            return undef;
        }
    } elsif ( 4 == $#_ ) {
        # Data validation by object accessor methods
        $local_logger->info( "Set alarm from parameter list." );
        my $als = ArduinoAlarms->new();
        unless( $alarm = $als->get_alarm( shift ) ) { 
            $local_logger->debug( "SetArarm-get_alarm failed.");
            return undef;
        }
        unless( defined( $alarm->hour( shift ) ) ) {
            $local_logger->debug( "SetArarm-hour failed.");
            return undef;
        }
        unless( defined( $alarm->minute( shift ) ) ) {
            $local_logger->debug( "SetArarm-minute failed.");
            return undef;
        }
        unless( defined( $alarm->second( shift ) ) ) {
            $local_logger->debug( "SetArarm-second failed.");
            return undef;
        }
        unless( defined( $alarm->day( shift ) ) ) {
            $local_logger->debug( "SetArarm-day failed.");
            return undef;
        }
        $local_logger->debug( "Parameter list parsed successfully." );
    } else {
        $local_logger->info( "Invalid number of arguments: $#_" );
        return undef;
    }

    # TODO
    # Have an alarm or alarms object in $alarm
    # Need to use it to set the alarm. Can use get_ar_json_str to
    # create request data. Need to create URI also. Maybe add
    # this to alarm/alarms classes!


    $local_logger->debug( "Preparing HTTP request URI." );
    my $req_uri = "/Alarms";
    if ( "ArduinoAlarm" eq ref( $_[0] ) ) {
        $req_uri .= "/" . $alarm->name;
    }
    $local_logger->info( "HTTP Post $req_uri" );
    $local_logger->debug( "JSON string: " . $alarm->get_ar_json_str );

    my $ua = LWP::UserAgent->new;
    my $server_endpoint = "http://" . $self->{IP_ADDR} . ${req_uri};
    my $req;
    $req = HTTP::Request->new( POST => $server_endpoint );

    $req->content( $alarm->get_ar_json_str );

    $local_logger->info( "HTTP POST $req_uri sending request..." );
    my $resp = $ua->request($req);


    if ( $resp->is_success ) {
        $local_logger->info( "HTTP POST $req_uri response received." );
        $local_logger->debug( "Response content: " . $resp->decoded_content );

        my $ret = $self->{ALARMS}->set_ar_json_str( $resp->decoded_content );

        if ( !defined( $ret ) ) {
            $local_logger->error( "Failed to update alarms object with HTTP response data." );
            $local_logger->debug( "HTTP response: " . $resp->decoded_content );
            return undef;
        }
        $local_logger->debug( "HTTP POST $req_uri response data:" );
        $local_logger->debug( Dumper( $self->{CLOCK} ) );

        return $ret;
    } else {
        $local_logger->error( "HTTP POST error code: ", $resp->code );
        $local_logger->error( "HTTP POST error message: ", $resp->message );
    }
}


1; # objects must return true
