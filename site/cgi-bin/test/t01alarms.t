#!/opt/local/bin/perl -w

use Test::Simple tests => 26; # seet to 1 more to make sure all are run

use Log::Log4perl qw( get_logger :levels);
use Data::Dumper;

use ArduinoAlarms;
use ArduinoAlarm;

Log::Log4perl->init( "perl_logger.conf" );
my $local_logger = get_logger( "ArduinoTest" );

my $t_alrs = ArduinoAlarms->new;
ok( defined($t_alrs),                'new() object constructor' );

my $al = $t_alrs->get_alarm( 0 );
ok( defined($al),                    'get_alarm( 0 ) reference to alarm member object' );
ok( "ArduinoAlarm" eq ref($al),      'get_alarm(), returned an ArduinoAlarm object' );
$al->hour(5);
$al->minute(6);
$al->second(7);
$al->day(8);
my $al2 = $t_alrs->get_alarm( 0 );
ok( 5 == $al2->hour,                    'alarm member object 0 hour set' );
ok( 6 == $al2->minute,                  'alarm member object 0 minute set' );
ok( 7 == $al2->second,                  'alarm member object 0 second set' );
ok( 8 == $al2->day,                     'alarm member object 0 day set' );
$al = $t_alrs->get_alarm( 1 );
$al->hour(4);
$al->minute(3);
$al->second(2);
$al->day(1);
$al = $t_alrs->get_alarm( 2 );
$al->hour(7);
$al->minute(8);
$al->second(9);
$al->day(0);
$al = $t_alrs->get_alarm( 3 );
$al->hour(9);
$al->minute(10);
$al->second(11);
$al->day(12);
my $ex_res = '{"Alarms":{"A0":"050607008","A1":"040302001","A2":"070809000","A3":"091011012"}}';

ok( $ex_res eq $t_alrs->get_ar_json_str,   'get_ar_json_str()' );

my $t_alrs2 = ArduinoAlarms->new;
my $set_val = '{"Alarms":{"A0":"010101001","A1":"020202002","A2":"030303003","A3":"040404004"}}';
$t_alrs2->set_ar_json_str( $set_val );
$al = $t_alrs2->get_alarm( 0 );

ok( defined($t_alrs2),                          'new() object constructor 2nd time' );
ok( "ArduinoAlarms" eq ref($t_alrs2),           'ArduinoAlarms object from new' );
ok( $set_val eq $t_alrs2->get_ar_json_str,       'set_ar_json_str() results check by get_ar_json_str()' );
ok( 1 == $al->hour,                             'set_ar_json_str() results check by Alarm object hour member accessor.' );
ok( 1 == $al->minute,                           'set_ar_json_str() results check by Alarm object minute member accessor.' );
ok( 1 == $al->second,                           'set_ar_json_str() results check by Alarm object second member accessor.' );
ok( 1 == $al->day,                              'set_ar_json_str() results check by Alarm object day member accessor.' );


my $res;
$t_alrs2 = ArduinoAlarms->new;
$set_val = '{"A0":"010101001"}';
$res = $t_alrs2->set_ar_json_str( $set_val );
ok( $set_val eq $res->get_ar_json_str,                      'set single alarm from JSON string 0 - resp.' );
ok( $set_val eq $t_alrs2->get_alarm(0)->get_ar_json_str,    'set single alarm from JSON string 0 - obj.' );
$set_val = '{"A1":"010101001"}';
$res = $t_alrs2->set_ar_json_str( $set_val );
ok( $set_val eq $res->get_ar_json_str,                      'set single alarm from JSON string 1 - resp.' );
ok( $set_val eq $t_alrs2->get_alarm(1)->get_ar_json_str,    'set single alarm from JSON string 1 - obj.' );
$set_val = '{"A2":"010101001"}';
$res = $t_alrs2->set_ar_json_str( $set_val );
ok( $set_val eq $res->get_ar_json_str,                      'set single alarm from JSON string 2 - resp.' );
ok( $set_val eq $t_alrs2->get_alarm(2)->get_ar_json_str,    'set single alarm from JSON string 2 - obj.' );
$set_val = '{"A3":"010101001"}';
$res = $t_alrs2->set_ar_json_str( $set_val );
ok( $set_val eq $res->get_ar_json_str,                      'set single alarm from JSON string 3 - resp.' );
ok( $set_val eq $t_alrs2->get_alarm(3)->get_ar_json_str,    'set single alarm from JSON string 3 - obj.' );

$t_alrs2 = ArduinoAlarms->new;
$set_val = '{"alarms":[{"hr":"1","mn":"2","sc":"3","dy":"4"},
                       {"hr":"2","mn":"3","sc":"4","dy":"5"},
                       {"hr":"3","mn":"4","sc":"5","dy":"6"},
                       {"hr":"4","mn":"5","sc":"6","dy":"1"}]}';
$res = $t_alrs2->set_ua_json_str( $set_val );
ok( defined( $t_alrs2),                         'set_ua_json_str(), success.' );
# TODO this test fails because returned data is
# not in same order, but results are ok.
ok( $s eq $t_alr->get_ua_json_str,                          'get_ua_json_str return eq set_ua_json_str parameter');
ok( $set_val eq $t_alrs2->get_ua_json_str,      'get_ua_json_str return eq set_ua_json_str parameter' );
$local_logger->debug( "UA JSON: " . $t_alrs2->get_ua_json_str );


