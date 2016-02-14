
package ArduinoClock;
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
    my $object_logger = get_logger( "ArduinoClock" );
    my $self = {
        HOUR => "",
        MINUTE => "",
        SECOND => "",
        DAY => "",
        OBJ_LOGGER => $object_logger,
        @_, # remaining arguments will override default values
    };
    bless( $self, $class );
    return $self;
}

# Set values to current local time
sub set_to_localtime
{
    # no arguments
    # localtime indexes:
    #   0 - second (0 based)
    #   1 - minute (0 based)
    #   2 - hour   (0 based)
    #   6 - day    (0 Sunday)

    my $self = shift;
    my @time_vals = localtime;
    $self->hour( $time_vals[2] );
    $self->minute( $time_vals[1] );
    $self->second( $time_vals[0] );
    $self->day( $time_vals[6] );
}


# Return Arduino JSON string for clock time
sub get_ar_json_str
{
    my $self = shift;

    my $time_str;
    $time_str = sprintf( "%02d", $self->hour );
    $time_str .= sprintf( "%02d", $self->minute );
    $time_str .= sprintf( "%02d", $self->second );
    $time_str .= sprintf( "%01d", $self->day );

    my $json_str = "{\"Clock\":\"$time_str\"}";

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
            $local_logger->warn( 'Failed to decode JSON string: ' . $json_str );
            return undef;
        }
        if ( exists( $json_data->{'Clock'} ) ) {
            # create a temp object and use accessor methods to validate data
            my $temp_obj = ArduinoClock->new();
            $local_logger->debug( 'JSON clock: ' . $json_data->{'Clock'} );
            defined( $temp_obj->hour(   substr( $json_data->{'Clock'}, 0, 2 ) ) ) || return undef;
            defined( $temp_obj->minute( substr( $json_data->{'Clock'}, 2, 2 ) ) ) || return undef;
            defined( $temp_obj->second( substr( $json_data->{'Clock'}, 4, 2 ) ) ) || return undef;
            defined( $temp_obj->day(    substr( $json_data->{'Clock'}, 6, 1 ) ) ) || return undef;

            $self->hour( $temp_obj->hour );
            $self->minute( $temp_obj->minute );
            $self->second( $temp_obj->second );
            $self->day( $temp_obj->day );

            return $self;
        } else {
            $local_logger->info( "Clock element not found in JSON data." );
            return undef;
        }
    } else {
        $local_logger->info( "Missing argument." );
        return undef;
    }
}

# Return user agent (browser) JSON string for clock time
sub get_ua_json_str
{
    my $self = shift;

    my $data = {};

    $data->{'clock'}->{'hr'} = $self->hour;
    $data->{'clock'}->{'mn'} = $self->minute;
    $data->{'clock'}->{'sc'} = $self->second;
    $data->{'clock'}->{'dy'} = $self->day;

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
        if ( exists( $json_data->{'clock'} ) ) {
            # create a temp object and use accessor methods to validate data
            my $temp_obj = ArduinoClock->new();
            $local_logger->debug( 'JSON ua clock: ' . Dumper( $json_data->{'clock'} ) );
            defined( $temp_obj->hour(   $json_data->{'clock'}->{'hr'} ) ) || return undef;
            defined( $temp_obj->minute( $json_data->{'clock'}->{'mn'} ) ) || return undef;
            defined( $temp_obj->second( $json_data->{'clock'}->{'sc'} ) ) || return undef;
            defined( $temp_obj->day(    $json_data->{'clock'}->{'dy'} ) ) || return undef;

            $self->hour( $temp_obj->hour );
            $self->minute( $temp_obj->minute );
            $self->second( $temp_obj->second );
            $self->day( $temp_obj->day );

            return $self;
        } else {
            $local_logger->info( "Clock element not found in ua JSON data." );
            return undef;
        }
    } else {
        $local_logger->info( "Missing argument to set_ua_json_str." );
        return undef;
    }
}

# HOUR accessor
sub hour
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    # $#_ returns subsricpt of last element of $_ array (@_ is the
    # parameter list) - if there is 1 parameter (0 == $#_) use it
    # to set the value of hour, with some validation
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
        if ( $l_dy < 7 ) {
            $self->{DAY} = $l_dy;
        } else {
            $local_logger->info( "Day value out of range." );
            return undef;
        }
    }
    
    return $self->{DAY};
}

1; # objects must return true
