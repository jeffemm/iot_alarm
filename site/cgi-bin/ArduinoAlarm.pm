
package ArduinoAlarm;
require Exporter;
use JSON;
use Log::Log4perl qw( get_logger :levels);
use Data::Dumper;

use strict;

our $VERSION = 1.00;
our @ISA = qw(Exporter);
our @EXPORT = ();
our @EXPORT_OK = qw();
our @EXPORT_TAGS = ();


# Object constructor
sub new
{
    my $invocant = shift; # first argument for class method is the invocant (class name or obj reference)
    my $class = ref($invocant) || $invocant;
    my $object_logger = get_logger( "ArduinoAlarm" );
    my $self;
    if ( @_ ) {
        $self = {
            HOUR => 0,
            MINUTE => 0,
            SECOND => 0,
            DAY => 0,
            NAME => "",
            VALID_NAMES => { 'A0' => "", 'A1' => "", 'A2' => "", 'A3' => "" },
            OBJ_LOGGER => $object_logger,
            @_,
        };
        if ( !exists( $self->{'VALID_NAMES'}{$self->{'NAME'}} ) ) {
            $object_logger->info( "Invalid alarm name in new construcutor." );
            return undef;
        }
        # TODO need to add unit test cases for constructor values
        if ( $self->{'HOUR'} > 23 ) {
            $object_logger->info( "Invalid alarm hour in new construcutor." );
            return undef;
        }
        if ( $self->{'MINUTE'} > 59 ) {
            $object_logger->info( "Invalid alarm minute in new construcutor." );
            return undef;
        }
        if ( $self->{'SECOND'} > 59 ) {
            $object_logger->info( "Invalid alarm second in new construcutor." );
            return undef;
        }
        if ( $self->{'DAY'} > 6 ) {
            $object_logger->info( "Invalid alarm day in new construcutor." );
            return undef;
        }
    } else {
        $object_logger->info( "No alarm name provided to new construcutor." );
        return undef;
    }
    bless( $self, $class );
    return $self;
}

# Return Arduino JSON string for alarm time
sub get_ar_json_str
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    my $time_str;
    $time_str = sprintf( "%02d", $self->hour );
    $time_str .= sprintf( "%02d", $self->minute );
    $time_str .= sprintf( "%02d", $self->second );
    $time_str .= sprintf( "%03d", $self->day );

    my $json_str = "{\"" . $self->name . "\":\"$time_str\"}";
    $local_logger->debug( "Alarm ar JSON string: \"", $json_str, "\"" );

    return $json_str;
}

# Set value from Arduino JSON string
#   returns object reference on success, or undef on failure
sub set_ar_json_str
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    if ( @_ ) {
        my $json_str = shift;
        my $json_data = eval { decode_json $json_str; };
        if ( $@ ) {
            $local_logger->warn( 'Failed to decode ar JSON string: ' . $json_str );
            return undef;
        }
        if ( exists( $json_data->{$self->name} ) ) {
            # create a temp object and use accessor methods to validate data
            my $temp_obj = ArduinoAlarm->new( NAME => "A0" );
            defined( $temp_obj->hour(   substr( $json_data->{$self->name}, 0, 2 ) ) ) || return undef;
            defined( $temp_obj->minute( substr( $json_data->{$self->name}, 2, 2 ) ) ) || return undef;
            defined( $temp_obj->second( substr( $json_data->{$self->name}, 4, 2 ) ) ) || return undef;
            defined( $temp_obj->day(    substr( $json_data->{$self->name}, 6, 3 ) ) ) || return undef;

            $self->hour( $temp_obj->hour );
            $self->minute( $temp_obj->minute );
            $self->second( $temp_obj->second );
            $self->day( $temp_obj->day );

            $local_logger->debug( Dumper( $temp_obj ) );

            return $self;
        } else {
            $local_logger->info( "Alarm element not found in ar JSON data." );
            return undef;
        }
    } else {
        $local_logger->info( "Alarm set from ar JSON missing argument." );
        return undef;
    }
}

# Return user agent (browser) JSON string for alarm time
sub get_ua_json_str
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    my $data = {};

    $data->{'hr'} = $self->hour;
    $data->{'mn'} = $self->minute;
    $data->{'sc'} = $self->second;
    $data->{'dy'} = $self->day;

    return encode_json( $data );
}

# Set value from user agent (browser) JSON string
#   returns object reference on success, or undef on failure
sub set_ua_json_str
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    if ( @_ ) {
        my $json_str = shift;
        my $json_data = eval { decode_json $json_str; };
        if ( $@ ) {
            $local_logger->warn( 'Failed to decode ua JSON string: ' . $json_str );
            return undef;
        }
        # create a temp object and use accessor methods to validate data
        my $temp_obj = ArduinoAlarm->new( NAME => "A0" );
        defined( $temp_obj->hour(   $json_data->{'hr'} ) ) || return undef;
        defined( $temp_obj->minute( $json_data->{'mn'} ) ) || return undef;
        defined( $temp_obj->second( $json_data->{'sc'} ) ) || return undef;
        defined( $temp_obj->day(    $json_data->{'dy'} ) ) || return undef;

        $self->hour( $temp_obj->hour );
        $self->minute( $temp_obj->minute );
        $self->second( $temp_obj->second );
        $self->day( $temp_obj->day );

        $local_logger->debug( Dumper( $temp_obj ) );

        return $self;
    } else {
        $local_logger->info( "Alarm set from ua JSON missing argument." );
        return undef;
    }
}

# NAME accessor
sub name
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    if ( 0 == $#_ ) {
        my $l_name = shift;
        # Name must be one of: A0, A1, A2, A3
        if ( exists( $self->{'VALID_NAMES'}{$l_name} ) ) {
            $self->{'NAME'} = $l_name;
        } else {
            $local_logger->info( "Name value is invalid." );
            return undef;
        }
    }
    
    return $self->{NAME};
}

# HOUR accessor
sub hour
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    if ( 0 == $#_ ) {
        my $l_hr = shift;
        if ( $l_hr < 24 ) {
            $self->{HOUR} = $l_hr;
        } else {
            $local_logger->info( "Hour value out of range." );
            return undef;
        }
    }
    
    return $self->{HOUR};
}

# MINUTE accessor
sub minute
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};
    
    if ( 0 == $#_ ) {
        my $l_mn = shift;
        if ( $l_mn < 60 ) {
            $self->{MINUTE} = $l_mn;
        } else {
            $local_logger->info( "Minute value out of range." );
            return undef;
        }
    }
    
    return $self->{MINUTE};
}

# SECOND accessor
sub second
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};
    
    if ( 0 == $#_ ) {
        my $l_sc = shift;
        if ( $l_sc < 60 ) {
            $self->{SECOND} = $l_sc;
        } else {
            $local_logger->info( "Second value out of range." );
            return undef;
        }
    }
    
    return $self->{SECOND};
}

# DAY accessor
sub day
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    if ( 0 == $#_ ) {
        my $l_dy = shift;
        if ( $l_dy < 0x80 ) {
            $self->{DAY} = $l_dy;
        } else {
            $local_logger->info( "Day value out of range." );
            return undef;
        }
    }
    
    return $self->{DAY};
}

1; # objects must return true
