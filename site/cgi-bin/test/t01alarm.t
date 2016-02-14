#!/opt/local/bin/perl -w

use Test::Simple tests => 50;

use Log::Log4perl qw( get_logger :levels);
use ArduinoAlarm;

Log::Log4perl->init( "perl_logger.conf" );
my $local_logger = get_logger( "ArduinoTest" );

my $t_alr = ArduinoAlarm->new( NAME => "A0" );
ok( defined($t_alr),                'new() object constructor' );
ok( "A0" eq $t_alr->name,           'name() get accessor' );

ok( 0 == $t_alr->hour(0),           'hour() set accessor, valid value 0' );
ok( 0 == $t_alr->minute(0),         'minute() set accessor, valid value 0' );
ok( 0 == $t_alr->second(0),         'second() set accessor, valid value 0' );
ok( 0 == $t_alr->day(0),            'day() set accessor, valid value 0' );

ok( 5 == $t_alr->hour(5),           'hour() set accessor, valid value' );
ok( 6 == $t_alr->minute(6),         'minute() set accessor, valid value' );
ok( 7 == $t_alr->second(7),         'second() set accessor, valid value' );
ok( 127 == $t_alr->day(127),        'day() set accessor, valid value' );

ok( !defined( $t_alr->hour(35) ),   'hour() set accessor, out of range value' );
ok( !defined( $t_alr->minute(73) ), 'minute() set accessor, out of range value' );
ok( !defined( $t_alr->second(80) ), 'second() set accessor, out of range value' );
ok( !defined( $t_alr->day(255) ),   'day() set accessor, out of range value' );

ok( 5 == $t_alr->hour,              'hour() get accessor' );
ok( 6 == $t_alr->minute,            'minute() get accessor' );
ok( 7 == $t_alr->second,            'second() get accessor' );
ok( 127 == $t_alr->day,             'day() get accessor' );

ok( '{"A0":"050607127"}' eq $t_alr->get_ar_json_str,                      'get_ar_json_string' );

ok( defined( $t_alr->set_ar_json_str( '{"A0":"111213006"}' ) ),       'set_ar_json_str(), valid input' );
ok( 11 == $t_alr->hour,                                                 'hour correct after set_ar_json_str' );
ok( 12 == $t_alr->minute,                                               'minute correct after set_ar_json_str' );
ok( 13 == $t_alr->second,                                               'second correct after set_ar_json_str' );
ok( 6 == $t_alr->day,                                                   'day correct after set_ar_json_str' );

ok( !defined( $t_alr->set_ar_json_str( '{"A0":"311213001"}' ) ),     'set_ar_json_str(), out of range hour' );
ok( !defined( $t_alr->set_ar_json_str( '{"A0":"118213001"}' ) ),     'set_ar_json_str(), out of range minute' );
ok( !defined( $t_alr->set_ar_json_str( '{"A0":"111293001"}' ) ),     'set_ar_json_str(), out of range second' );
ok( !defined( $t_alr->set_ar_json_str( '{"A0":"111213130"}' ) ),     'set_ar_json_str(), out of range day' );
ok( !defined( $t_alr->set_ar_json_str( '{"A1":"111213001"}' ) ),     'set_ar_json_str(), invalid element name' );
ok( !defined( $t_alr->set_ar_json_str( '"A0":"1112139"}' ) ),     'set_ar_json_str(), invalid json string' );

ok( 11 == $t_alr->hour,                                                 'hour not modified by invalid set_ar_json_str' );
ok( 12 == $t_alr->minute,                                               'minute not modified by invalid set_ar_json_str' );
ok( 13 == $t_alr->second,                                               'second not modified by invalid set_ar_json_str' );
ok( 6 == $t_alr->day,                                                   'day not modified by invalid set_ar_json_str' );


my $s = '{"A0":"000000000"}';
ok( defined( $t_alr->set_ar_json_str( $s ) ),                         'set_ar_json_str(), call, test value ' . $s );
ok( $s eq $t_alr->get_ar_json_str,                                      'set_ar_json_str(), check, test value ' . $s );
$s = '{"A0":"235959127"}';
ok( defined( $t_alr->set_ar_json_str( $s ) ),                         'set_ar_json_str(), call, test value ' . $s );
ok( $s eq $t_alr->get_ar_json_str,                                      'set_ar_json_str(), check, test value ' . $s );

ok( !defined( ArduinoAlarm->new( NAME => "A4" ) ),                              'constructor with invalid name.' );
ok( !defined( $t_alr->name( "xx" ) ),                                   'set name with invalid value.' );
ok( "A0" eq $t_alr->name,                                               'invalid set name did not change name value' );

ok( defined( ArduinoAlarm->new( NAME => "A0",
                                HOUR => 0,
                                MINUTE =>0,
                                SECOND => 0,
                                DAY => 0 ) ),               'constructor with all values valid.' );
ok( !defined( ArduinoAlarm->new( NAME => "A0",
                                HOUR => 24,
                                MINUTE =>0,
                                SECOND => 0,
                                DAY => 0 ) ),               'constructor with all values, invalid hour.' );
ok( !defined( ArduinoAlarm->new( NAME => "A0",
                                HOUR => 0,
                                MINUTE =>60,
                                SECOND => 0,
                                DAY => 0 ) ),               'constructor with all values, invalid minute.' );
ok( !defined( ArduinoAlarm->new( NAME => "A0",
                                HOUR => 0,
                                MINUTE =>0,
                                SECOND => 60,
                                DAY => 0 ) ),               'constructor with all values, invalid second.' );
ok( !defined( ArduinoAlarm->new( NAME => "A0",
                                HOUR => 0,
                                MINUTE =>0,
                                SECOND => 0,
                                DAY => 7 ) ),               'constructor with all values, invalid day.' );

$t_alr = ArduinoAlarm->new( NAME => "A0" );
$s = '{"hr":"3","mn":"4","sc":"6","dy":"8"}';
ok( defined( $t_alr->set_ua_json_str( $s ) ),               'set_ua_json_str(), valid input' );
# TODO this test fails because returned data is
# not in same order, but results are ok.
ok( $s eq $t_alr->get_ua_json_str,                          'get_ua_json_str return eq set_ua_json_str parameter');
$local_logger->debug( "UA JSON: " . $t_alr->get_ua_json_str );





