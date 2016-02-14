
package ArduinoAlarms;
require Exporter;
use JSON;
use Log::Log4perl qw( get_logger :levels);
use Data::Dumper;
use ArduinoAlarm;

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
    my $object_logger = get_logger( "ArduinoAlarms" );
    my $self = {
        ALL_ALARMS => [ ],
        OBJ_LOGGER => $object_logger,
    };

    push( @{$self->{ALL_ALARMS}}, ArduinoAlarm->new( NAME => "A0" ) );
    push( @{$self->{ALL_ALARMS}}, ArduinoAlarm->new( NAME => "A1" ) );
    push( @{$self->{ALL_ALARMS}}, ArduinoAlarm->new( NAME => "A2" ) );
    push( @{$self->{ALL_ALARMS}}, ArduinoAlarm->new( NAME => "A3" ) );

    
    bless( $self, $class );
    return $self;
}

# Return Arduino JSON string for alarm time
sub get_ar_json_str
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    my $s;
    my $json_str = '{"Alarms":{';
    $s = $self->{ALL_ALARMS}->[0]->get_ar_json_str;
    $s =~ s/^{|}$//g;
    $json_str .= $s;
    $json_str .= ',';
    $s = $self->{ALL_ALARMS}->[1]->get_ar_json_str;
    $s =~ s/^{|}$//g;
    $json_str .= $s;
    $json_str .= ',';
    $s = $self->{ALL_ALARMS}->[2]->get_ar_json_str;
    $s =~ s/^{|}$//g;
    $json_str .= $s;
    $json_str .= ',';
    $s = $self->{ALL_ALARMS}->[3]->get_ar_json_str;
    $s =~ s/^{|}$//g;
    $json_str .= $s;
    $json_str .= '}}';

    $local_logger->debug( 'JSON ar string: ' . $json_str );
    return $json_str;
}

# Set value from Arduino JSON string
#   returns object reference on success, or undef on failure
#   works for JSON string for one alarm or all alarms
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
        if ( exists( $json_data->{'Alarms'} ) ) {
            # Need to set each alarm object with the correct element of JSON
            # data, based on alarm objects name.
            #$local_logger->debug( Dumper( $self ) );
            for ( my $i = 0; $i < 4; $i++ ) {
                my $j_str = '{"' . $self->{'ALL_ALARMS'}->[$i]->name . '":';
                $j_str .= '"' . $json_data->{'Alarms'}->{$self->{'ALL_ALARMS'}->[$i]->name} . '"}';
                $self->{'ALL_ALARMS'}->[$i]->set_ar_json_str( $j_str ) || return undef;
            }
            $local_logger->debug( Dumper( $self ) );

            return $self;
        } else {
            # Check for individual alarm value
            $local_logger->debug( "Alarms dump: " . Dumper( $self ) );
            for ( my $i = 0; $i < 4; $i++ ) {
                if ( exists( $json_data->{ $self->{'ALL_ALARMS'}->[$i]->name } ) ) {
                    $local_logger->debug( "Set single ar alarm: " . Dumper( $json_data ) );
                    $self->{'ALL_ALARMS'}->[$i]->set_ar_json_str( $json_str ) || return undef;
                    return $self->{'ALL_ALARMS'}->[$i];
                }
            }
            
            $local_logger->info( "Alarm(s) element not found in ar JSON data." );
            return undef;
        }
    } else {
        $local_logger->info( "Alarms set from ar JSON missing argument." );
        return undef;
    }
}


# Return user agent (browser) JSON string for alarm time
sub get_ua_json_str
{
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    my $s;
    my $json_str = '{"alarms":[';
    $json_str .= $self->{ALL_ALARMS}->[0]->get_ua_json_str;
    $json_str .= ',';
    $json_str .= $self->{ALL_ALARMS}->[1]->get_ua_json_str;
    $json_str .= ',';
    $json_str .= $self->{ALL_ALARMS}->[2]->get_ua_json_str;
    $json_str .= ',';
    $json_str .= $self->{ALL_ALARMS}->[3]->get_ua_json_str;
    $json_str .= ']}';

    $local_logger->debug( 'JSON ua string: ' . $json_str );
    return $json_str;
}

# Set value from user agent (browser) JSON string
#   returns object reference on success, or undef on failure
#   works for JSON string for all alarms only.
#   Need to use get_alarm, then call set_ua_json_str for the
#   ArduinoAlarm object to set one alarm
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
        if ( exists( $json_data->{'alarms'} ) ) {
            # Set all alarms
            for ( my $i = 0; $i < 4; $i++ ) {
                # TODO this will set some alarms if one fails
                # Should not set any if any fail? Or set all that it can?
                unless( defined( $self->{'ALL_ALARMS'}->[$i]->set_ua_json_str(
                  encode_json( $json_data->{'alarms'}->[$i] ) ) ) ) {
                    $local_logger->info( "Failed to set alarm $i from ua JSON" );
                    return undef;
                }
            }
            return $self;
        } else {
            $local_logger->info( "Alarm(s) element not found in ua JSON data." );
            return undef;
        }
    } else {
        $local_logger->info( "Alarms set from ua JSON missing argument." );
        return undef;
    }
}

sub get_alarm
{
    # Arguments
    #   0-3 - index of alarm to get
    # Returns alarm object or undef on error
    my $self = shift;
    my $local_logger = $self->{OBJ_LOGGER};

    if ( @_ ) {
        if ( defined($_[0]) && ($_[0] < 4) ) {
            $local_logger->debug( "Alarms get alarm." );
            return $self->{ALL_ALARMS}->[$_[0]];
        } else {
            $local_logger->info( "Alarms get alarm only has 4 alarms." );
            return undef;
        }
    } else {
        $local_logger->info( "Alarms get alarm requires an arguement." );
        return undef;
    }
}

1; # objects must return true
