#!/opt/local/bin/perl -w

use Test::More;
use JSON;

use Log::Log4perl qw( get_logger :levels);
use ArduinoInterface;
use ArduinoAlarms;
use ArduinoAlarm;

Log::Log4perl->init( "perl_logger.conf" );

my $skip_timeout = 1; # skip test that need to wait for a timeout

my $ip = "192.168.0.199";
my $bad_ip = "192.168.0.187";

my $intf;
$intf = ArduinoInterface->new;
ok( defined( $intf ),                   'new() object constructor, no IP address' );
ok( !defined( $intf->GetArduinoTime ),  'GetArduinoTime() method with no IP address' );
ok( !defined( $intf->GetArduinoAlarm ),  'GetArduinoAlarm() method with no IP address' );


$intf = ArduinoInterface->new( IP_ADDR => $ip );
ok( "ArduinoClock" eq ref( $intf->GetArduinoTime ), 'GetArduinoTime method with IP' );
ok( "ArduinoAlarms" eq ref( $intf->GetArduinoAlarm ), 'GetArduinoAlarm method with IP' );
ok( "ArduinoAlarm" eq ref( $intf->GetArduinoAlarm( 0 ) ), 'GetArduinoAlarm 0 method with IP' );
ok( "ArduinoAlarm" eq ref( $intf->GetArduinoAlarm( 1 ) ), 'GetArduinoAlarm 1 method with IP' );
ok( "ArduinoAlarm" eq ref( $intf->GetArduinoAlarm( 2 ) ), 'GetArduinoAlarm 2 method with IP' );
ok( "ArduinoAlarm" eq ref( $intf->GetArduinoAlarm( 3 ) ), 'GetArduinoAlarm 3 method with IP' );
ok( !defined( $intf->GetArduinoAlarm( 4 ) ), 'GetArduinoAlarm 4 method with IP, invalid alarm index' );

SKIP: {
    skip "Test take a long time, disabled to speed up dev/debug.", 2 if $skip_timeout;

    $intf = ArduinoInterface->new( IP_ADDR => $bad_ip );
    ok( !defined( $intf->GetArduinoTime ), 'GetArduinoTime method with wrong IP' );
    ok( !defined( $intf->GetArduinoAlarm ), 'GetArduinoAlarm method with wrong IP' );
}

$intf = ArduinoInterface->new( IP_ADDR => $ip );
ok( defined( $intf->SetArduinoTime ),   'SetArduinoTime method, no arguments - localtime' );
ok( defined( $intf->SetArduinoTime( 11, 12, 13, 4 ) ), 'SetArduinoTime method, valid argument list' );
ok( defined( $intf->SetArduinoTime( 0, 0, 0, 0 ) ), 'SetArduinoTime method, valid argument list, zeros' );

my $clk = ArduinoClock->new( HOUR => 10, MINUTE => 9, SECOND => 8, DAY => 3 );
ok( defined( $intf->SetArduinoTime( $clk ) ),   'SetArduinoTime method, object reference' );
$clk = ArduinoAlarm->new( NAME => "A0" );
ok( !defined( $intf->SetArduinoTime( $clk ) ),   'SetArduinoTime method, invalid object reference' );
ok( !defined( $intf->SetArduinoTime( 24, 0, 0, 0 ) ), 'SetArduinoTime method, invalid valid argument list, hour' );
ok( !defined( $intf->SetArduinoTime( 0, 60, 0, 0 ) ), 'SetArduinoTime method, invalid valid argument list, minute' );
ok( !defined( $intf->SetArduinoTime( 0, 0, 60, 0 ) ), 'SetArduinoTime method, invalid valid argument list, second' );
ok( !defined( $intf->SetArduinoTime( 0, 0, 0, 7 ) ), 'SetArduinoTime method, invalid valid argument list, day' );

my $alr = ArduinoAlarm->new( NAME => "A0", HOUR => 0, MINUTE => 0, SECOND => 0, DAY => 0 ); 
ok( defined( $intf->SetArduinoAlarm( $alr ) ),   'SetArduinoAlarm method, alarm object arguement' );
ok( defined( $intf->SetArduinoAlarm( 0, 0, 0, 0, 0 ) ), 'SetArduinoAlarm method, list arguements' );
ok( !defined( $intf->SetArduinoAlarm( 4, 0, 0, 0, 0 ) ), 'SetArduinoAlarm method, invalid list num arguement value' );
ok( !defined( $intf->SetArduinoAlarm( 0, 24, 0, 0, 0 ) ), 'SetArduinoAlarm method, invalid list hour arguement value' );
ok( !defined( $intf->SetArduinoAlarm( 0, 0, 60, 0, 0 ) ), 'SetArduinoAlarm method, invalid list minute arguement value' );
ok( !defined( $intf->SetArduinoAlarm( 0, 0, 0, 60, 0 ) ), 'SetArduinoAlarm method, invalid list second arguement value' );
ok( !defined( $intf->SetArduinoAlarm( 0, 0, 0, 0, 128 ) ), 'SetArduinoAlarm method, invalid list day arguement value' );

$alrs = ArduinoAlarms->new;
ok( defined( $intf->SetArduinoAlarm( $alrs ) ), 'SetArduinoAlarm method, alarms object arguement' );
$alrs = ArduinoClock->new;
ok( !defined( $intf->SetArduinoAlarm( $alrs ) ), 'SetArduinoAlarm method, invalid object arguement' );
ok( !defined( $intf->SetArduinoAlarm() ), 'SetArduinoAlarm method, no arguements' );

# Test GetArduinoAll
ok( defined( $intf->GetArduinoAll ),       'GetArduinoAll: returned defined value' );

# Test SetArduinoAll
my $d = {};
$d->{'clock'}->{'hr'} = 0;
$d->{'clock'}->{'mn'} = 0;
$d->{'clock'}->{'sc'} = 0;
$d->{'clock'}->{'dy'} = 0;
$d->{'alarms'}->[0] = { hr => 0, mn => 0, sc => 0, dy => 0 };
$d->{'alarms'}->[1] = { hr => 0, mn => 0, sc => 0, dy => 0 };
$d->{'alarms'}->[2] = { hr => 0, mn => 0, sc => 0, dy => 0 };
$d->{'alarms'}->[3] = { hr => 0, mn => 0, sc => 0, dy => 0 };
my $s = encode_json $d;
ok( defined( $intf->SetArduinoAll( $s ) ),       'SetArduinoAll: returned defined value' );



################################################################################
done_testing();
