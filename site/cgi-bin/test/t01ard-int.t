#!/opt/local/bin/perl -w

use Log::Log4perl qw( get_logger :levels);
use Data::Dumper;
use LWP::UserAgent;
use JSON;

use Test::Simple tests => 124;


Log::Log4perl->init( "perl_logger.conf" );
my $local_logger = get_logger( "ArduinoTest" );

my $host = "localhost";
my $ua = LWP::UserAgent->new;
my $server_endpoint;
my $req;
my $post_str;
my $resp;
my $resp_data;
my $json_data;


# Get Everything
$server_endpoint = "http://$host/~jje/site/api";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET / -> response success');           #    1
ok( $resp_data = $resp->decoded_content,        'GET / -> decoded_content');            #    2
#$local_logger->debug( "JSON response string: $resp_data" );
ok( $json_data = decode_json($resp_data),       'GET / -> decode_json');                #    3
#$local_logger->debug( "JSON response object: ", Dumper($json_data) );
ok( exists( $json_data->{'clock'} ),            'GET / -> clock in json');              #    4
ok( exists( $json_data->{'alarms'} ),           'GET / -> alarms in json');             #    5
ok( 4 == scalar( @{$json_data->{'alarms'}} ),   'GET / -> alarms array size');          #    6
my $temp = $json_data->{'alarms'}->[0]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET / -> alarms element 0 hr');        #    7
ok( exists( $temp->{'mn'} ),                    'GET / -> alarms element 0 mn');        #    8
ok( exists( $temp->{'sc'} ),                    'GET / -> alarms element 0 sc');        #    9
ok( exists( $temp->{'dy'} ),                    'GET / -> alarms element 0 dy');        #   10
$temp = $json_data->{'alarms'}->[1]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET / -> alarms element 1 hr');        #   11
ok( exists( $temp->{'mn'} ),                    'GET / -> alarms element 1 mn');        #   12
ok( exists( $temp->{'sc'} ),                    'GET / -> alarms element 1 sc');        #   13
ok( exists( $temp->{'dy'} ),                    'GET / -> alarms element 1 dy');        #   14
$temp = $json_data->{'alarms'}->[2]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET / -> alarms element 2 hr');        #   15
ok( exists( $temp->{'mn'} ),                    'GET / -> alarms element 2 mn');        #   16
ok( exists( $temp->{'sc'} ),                    'GET / -> alarms element 2 sc');        #   17
ok( exists( $temp->{'dy'} ),                    'GET / -> alarms element 2 dy');        #   18
$temp = $json_data->{'alarms'}->[3]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET / -> alarms element 3 hr');        #   19
ok( exists( $temp->{'mn'} ),                    'GET / -> alarms element 3 mn');        #   20
ok( exists( $temp->{'sc'} ),                    'GET / -> alarms element 3 sc');        #   21
ok( exists( $temp->{'dy'} ),                    'GET / -> alarms element 3 dy');        #   22
$temp = $json_data->{'clock'}; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET / -> clock element hr');           #   23
ok( exists( $temp->{'mn'} ),                    'GET / -> clock element mn');           #   24
ok( exists( $temp->{'sc'} ),                    'GET / -> clock element sc');           #   25
ok( exists( $temp->{'dy'} ),                    'GET / -> clock element dy');           #   26

# Get All Alarms
$server_endpoint = "http://$host/~jje/site/api/alarms";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET /alarms -> response success');           #   27
ok( $resp_data = $resp->decoded_content,        'GET /alarms -> decoded_content');            #   28
ok( $json_data = decode_json($resp_data),       'GET /alarms -> decode_json');                #   29
ok( exists( $json_data->{'alarms'} ),           'GET /alarms -> alarms in json');             #   30
ok( 4 == scalar( @{$json_data->{'alarms'}} ),   'GET /alarms -> alarms array size');          #   31
$temp = $json_data->{'alarms'}->[0]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET /alarms -> alarms element 0 hr');        #   32
ok( exists( $temp->{'mn'} ),                    'GET /alarms -> alarms element 0 mn');        #   33
ok( exists( $temp->{'sc'} ),                    'GET /alarms -> alarms element 0 sc');        #   34
ok( exists( $temp->{'dy'} ),                    'GET /alarms -> alarms element 0 dy');        #   35
$temp = $json_data->{'alarms'}->[1]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET /alarms -> alarms element 1 hr');        #   36
ok( exists( $temp->{'mn'} ),                    'GET /alarms -> alarms element 1 mn');        #   37
ok( exists( $temp->{'sc'} ),                    'GET /alarms -> alarms element 1 sc');        #   38
ok( exists( $temp->{'dy'} ),                    'GET /alarms -> alarms element 1 dy');        #   39
$temp = $json_data->{'alarms'}->[2]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET /alarms -> alarms element 2 hr');        #   40
ok( exists( $temp->{'mn'} ),                    'GET /alarms -> alarms element 2 mn');        #   41
ok( exists( $temp->{'sc'} ),                    'GET /alarms -> alarms element 2 sc');        #   42
ok( exists( $temp->{'dy'} ),                    'GET /alarms -> alarms element 2 dy');        #   43
$temp = $json_data->{'alarms'}->[3]; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET /alarms -> alarms element 3 hr');        #   44
ok( exists( $temp->{'mn'} ),                    'GET /alarms -> alarms element 3 mn');        #   45
ok( exists( $temp->{'sc'} ),                    'GET /alarms -> alarms element 3 sc');        #   46
ok( exists( $temp->{'dy'} ),                    'GET /alarms -> alarms element 3 dy');        #   47

# Get Alarm 0
$server_endpoint = "http://$host/~jje/site/api/alarms/0";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET /alarms/0 -> response success');           #   48
ok( $resp_data = $resp->decoded_content,        'GET /alarms/0 -> decoded_content');            #   49
ok( $json_data = decode_json($resp_data),       'GET /alarms/0 -> decode_json');                #   50
ok( 1,                                          'test removed');                                #   51
ok( 1,                                          'test removed');                                #   52
ok( exists( $json_data->{'hr'} ),               'GET /alarms/0 -> alarms element hr');          #   53
ok( exists( $json_data->{'mn'} ),               'GET /alarms/0 -> alarms element mn');          #   54
ok( exists( $json_data->{'sc'} ),               'GET /alarms/0 -> alarms element sc');          #   55
ok( exists( $json_data->{'dy'} ),               'GET /alarms/0 -> alarms element dy');          #   56

# Get Alarm 1
$server_endpoint = "http://$host/~jje/site/api/alarms/1";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET /alarms/1 -> response success');           #   57
ok( $resp_data = $resp->decoded_content,        'GET /alarms/1 -> decoded_content');            #   58
ok( $json_data = decode_json($resp_data),       'GET /alarms/1 -> decode_json');                #   59
ok( 1,                                          'test removed');                                #   60
ok( 1,                                          'test removed');                                #   61
ok( exists( $json_data->{'hr'} ),               'GET /alarms/1 -> alarms element hr');          #   62
ok( exists( $json_data->{'mn'} ),               'GET /alarms/1 -> alarms element mn');          #   63
ok( exists( $json_data->{'sc'} ),               'GET /alarms/1 -> alarms element sc');          #   64
ok( exists( $json_data->{'dy'} ),               'GET /alarms/1 -> alarms element dy');          #   65

# Get Alarm 2
$server_endpoint = "http://$host/~jje/site/api/alarms/2";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET /alarms/2 -> response success');           #   66
ok( $resp_data = $resp->decoded_content,        'GET /alarms/2 -> decoded_content');            #   67
ok( $json_data = decode_json($resp_data),       'GET /alarms/2 -> decode_json');                #   68
ok( 1,                                          'test removed');                                #   69
ok( 1,                                          'test removed');                                #   70
ok( exists( $json_data->{'hr'} ),               'GET /alarms/2 -> alarms element hr');          #   71
ok( exists( $json_data->{'mn'} ),               'GET /alarms/2 -> alarms element mn');          #   72
ok( exists( $json_data->{'sc'} ),               'GET /alarms/2 -> alarms element sc');          #   73
ok( exists( $json_data->{'dy'} ),               'GET /alarms/2 -> alarms element dy');          #   74

# Get Alarm 3
$server_endpoint = "http://$host/~jje/site/api/alarms/3";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET /alarms/3 -> response success');           #   75
ok( $resp_data = $resp->decoded_content,        'GET /alarms/3 -> decoded_content');            #   76
ok( $json_data = decode_json($resp_data),       'GET /alarms/3 -> decode_json');                #   77
ok( 1,                                          'test removed');                                #   78
ok( 1,                                          'test removed');                                #   79
ok( exists( $json_data->{'hr'} ),               'GET /alarms/3 -> alarms element hr');          #   80
ok( exists( $json_data->{'mn'} ),               'GET /alarms/3 -> alarms element mn');          #   81
ok( exists( $json_data->{'sc'} ),               'GET /alarms/3 -> alarms element sc');          #   82
ok( exists( $json_data->{'dy'} ),               'GET /alarms/3 -> alarms element dy');          #   83

# Get clock
$server_endpoint = "http://$host/~jje/site/api/clock";
$req = HTTP::Request->new( GET => $server_endpoint );
$req->header('content-type' => 'application/json');
$resp = $ua->request($req);
ok( $resp->is_success,                          'GET /clock -> response success');           #   84
ok( $resp_data = $resp->decoded_content,        'GET /clock -> decoded_content');            #   85
ok( $json_data = decode_json($resp_data),       'GET /clock -> decode_json');                #   86
ok( exists( $json_data->{'clock'} ),            'GET /clock -> clock in json');              #   87
$temp = $json_data->{'clock'}; # to shorten following lines
ok( exists( $temp->{'hr'} ),                    'GET /clock -> clock element hr');           #   88
ok( exists( $temp->{'mn'} ),                    'GET /clock -> clock element mn');           #   89
ok( exists( $temp->{'sc'} ),                    'GET /clock -> clock element sc');           #   90
ok( exists( $temp->{'dy'} ),                    'GET /clock -> clock element dy');           #   91

# Set clock
my $clock = {};
$clock->{'clock'}->{'hr'} = 0;
$clock->{'clock'}->{'mn'} = 0;
$clock->{'clock'}->{'sc'} = 0;
$clock->{'clock'}->{'dy'} = 0;
$server_endpoint = "http://$host/~jje/site/api/clock";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $clock ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /clock -> response success');           #   92
ok( $resp_data = $resp->decoded_content,        'POST /clock -> decoded_content');            #   93
$local_logger->debug( "POST /clock response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /clock -> decode_json');                #   94
ok( exists( $json_data->{'clock'} ),            'POST /clock -> clock in json');              #   95

# Set alarm/0
my $alarm = {};
$alarm->{'hr'} = 0;
$alarm->{'mn'} = 0;
$alarm->{'sc'} = 0;
$alarm->{'dy'} = 0;
$server_endpoint = "http://$host/~jje/site/api/alarms/0";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $alarm ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /alarms/0 -> response success');           #   96
ok( $resp_data = $resp->decoded_content,        'POST /alarms/0 -> decoded_content');            #   97
$local_logger->debug( "POST /alarms/0 response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /alarms/0 -> decode_json');                #   98
ok( 1,                                          'test removed');                                 #   99

# Set alarm/1
$alarm = {};
$alarm->{'hr'} = 0;
$alarm->{'mn'} = 0;
$alarm->{'sc'} = 0;
$alarm->{'dy'} = 0;
$server_endpoint = "http://$host/~jje/site/api/alarms/1";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $alarm ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /alarms/1 -> response success');           #   100
ok( $resp_data = $resp->decoded_content,        'POST /alarms/1 -> decoded_content');            #   101
$local_logger->debug( "POST /alarms/1 response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /alarms/1 -> decode_json');                #   102
ok( 1,                                          'test removed');                                 #   103

# Set alarm/2
$alarm = {};
$alarm->{'hr'} = 0;
$alarm->{'mn'} = 0;
$alarm->{'sc'} = 0;
$alarm->{'dy'} = 0;
$server_endpoint = "http://$host/~jje/site/api/alarms/2";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $alarm ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /alarms/2 -> response success');           #  104
ok( $resp_data = $resp->decoded_content,        'POST /alarms/2 -> decoded_content');            #  105
$local_logger->debug( "POST /alarms/2 response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /alarms/2 -> decode_json');                #  106
ok( 1,                                          'test removed');                                 #  107

# Set alarm/3
$alarm = {};
$alarm->{'hr'} = 0;
$alarm->{'mn'} = 0;
$alarm->{'sc'} = 0;
$alarm->{'dy'} = 0;
$server_endpoint = "http://$host/~jje/site/api/alarms/3";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $alarm ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /alarms/3 -> response success');           #  108
ok( $resp_data = $resp->decoded_content,        'POST /alarms/3 -> decoded_content');            #  109
$local_logger->debug( "POST /alarms/3 response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /alarms/3 -> decode_json');                #  110
ok( 1,                                          'test removed');                                 #  111

# Set alarms
$alarm = {};
for ( my $i = 0; $i < 4; $i++ ) {
    $alarm->{'alarms'}->[$i]->{'hr'} = 0;
    $alarm->{'alarms'}->[$i]->{'mn'} = 0;
    $alarm->{'alarms'}->[$i]->{'sc'} = 0;
    $alarm->{'alarms'}->[$i]->{'dy'} = 0;
}
$server_endpoint = "http://$host/~jje/site/api/alarms";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $alarm ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /alarms -> response success');           #  112
ok( $resp_data = $resp->decoded_content,        'POST /alarms -> decoded_content');            #  113
$local_logger->debug( "POST /alarms response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /alarms -> decode_json');                #  114
ok( exists( $json_data->{'alarms'} ),           'POST /alarms -> clock in json');              #  115
ok( 4 == scalar( @{$json_data->{'alarms'}} ),   'POST /alarms -> alarms array size');          #  116

# Set clock localtime
$clock = {};
$clock->{'clock'} = 'localtime';
$server_endpoint = "http://$host/~jje/site/api/clock";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $clock ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /clock localtime -> response success');  #  117
ok( $resp_data = $resp->decoded_content,        'POST /clock localtime -> decoded_content');   #  118
$local_logger->debug( "POST /clock response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /clock localtime -> decode_json');       #  119
ok( exists( $json_data->{'clock'} ),            'POST /clock localtime -> clock in json');     #  120

# Set clock localtime
my $clock = {};
$clock->{'clock'} = "localtime";
$server_endpoint = "http://$host/~jje/site/api/clock";
$req = HTTP::Request->new( POST => $server_endpoint );
$req->header('content-type' => 'application/json');
$req->content( encode_json( $clock ) );
$resp = $ua->request($req);
ok( $resp->is_success,                          'POST /clock -> response success');           #   121
ok( $resp_data = $resp->decoded_content,        'POST /clock -> decoded_content');            #   122
$local_logger->debug( "POST /clock response content:\n $resp_data" );
ok( $json_data = decode_json($resp_data),       'POST /clock -> decode_json');                #   123
ok( exists( $json_data->{'clock'} ),            'POST /clock -> clock in json');              #   124



