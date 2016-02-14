

function addEventHandlers() {
    document.getElementById("clock_set").addEventListener('click', SetArduinoLocalTime, false);
    document.getElementById("clock_refresh").addEventListener('click',
            function() { GetArduinoItem( "/clock" ); }, false);
    document.getElementById("alarm_refresh").addEventListener('click',
            function() { GetArduinoItem( "/alarms" ); }, false);
    document.getElementById("alarm_set").addEventListener('click', SetArduinoAlarm, false);
}

function SetArduinoAlarm( e ) {
    var frm = document.getElementById('alarm_control');
    var day = 0;
    var days = frm.elements['alarm_day'];
    for ( var i = 0; i < days.length; i++ ) {
        if ( days[i].checked ) {
            day += parseInt( days[i].value );
        }
    }
    var data = { 'hr': frm.elements['alarm_hour'].value,
                 'mn': frm.elements['alarm_minute'].value,
                 'sc': frm.elements['alarm_second'].value,
                 'dy': day.toString() };
    var num = frm.elements['alarm_index'].value;

    var request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (this.readyState == 4) {
            if (this.status == 200) {
                if (this.responseText != null) {
                    var data = JSON.parse( this.responseText );
                    UpdateAlarmDisplay( data, num );
                }
            }
        }
    }
    request.open( "POST", "/~jje/site/api/alarms/" + num, true );
    request.setRequestHeader("Content-Type",
            'application/json');
    request.send( JSON.stringify( data ) );
}

function UpdateAlarmDisplay( data, num ) {
    var alarms = document.getElementById("alarms").getElementsByTagName("div");
    for ( var i = 0; i < alarms.length; i++ ) {
        if ( alarms[i].hasAttribute( "data-alarm-index" ) ) {
            if ( num == parseInt( alarms[i].attributes['data-alarm-index'].value ) ) {
                alarms[i].getElementsByClassName("atm")[0].innerHTML =
                    FormatClockTime( data );
                el = alarms[i].getElementsByClassName("aday");
                for ( var j = 0; j < el.length; j++ ) {
                    if ( AlarmDayEnabled( data, el[j] ) ) {
                        el[j].className = "aday don";
                    } else {
                        el[j].className = "aday doff";
                    }
                }
            }
        }
    }
}

function UpdateArduinoDisplay( data ) {
    if ( 'sw_out' in data ) {
        document.getElementById("TODO").innerHTML = data.sw_out;
    }
    if ( 'clock' in data ) {
        document.getElementById("clock_day").innerHTML = DayNum2Name( data.clock.dy );
        document.getElementById("clock_time").innerHTML = FormatClockTime( data.clock );
    }
    if ( 'alarms' in data ) {
        // TODO how to setup HTML and access alarms and alarm parts as
        // an array or other structure
        // Should be able to get a node list. Add id to outer div
        // and maybe use comment id's for child nodes.
        //
        // document.getElementById only returns first element that matches id
        // document.getElementsByName("name") returns array of elements that match name
        // element.childNodes returns a collection of nodes as a node list

        var elAlarms = document.getElementById("alarms").getElementsByTagName("div");
        for ( var i = 0; i < elAlarms.length; i++ ) {
            var el = elAlarms[i].getElementsByClassName("atm");
            // There should be only one of these
            el[0].innerHTML = FormatClockTime( data.alarms[i] );
            el = elAlarms[i].getElementsByClassName("aday");
            for ( var j = 0; j < el.length; j++ ) {
                if ( AlarmDayEnabled( data.alarms[i], el[j] ) ) {
                    el[j].className = "aday don";
                } else {
                    el[j].className = "aday doff";
                }
            }
        }
    }
}

function SetArduinoLocalTime() {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (this.readyState == 4) {
            if (this.status == 200) {
                if (this.responseText != null) {
                    var data = JSON.parse( this.responseText );
                    UpdateArduinoDisplay( data );
                }
            }
        }
    }
    request.open( "POST", "/~jje/site/api/clock", true );
    request.setRequestHeader("Content-Type",
            'application/json');
    request.send( JSON.stringify( {'clock':'localtime'} ) );
}

function GetArduinoItem( item ) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (this.readyState == 4) {
            if (this.status == 200) {
                if (this.responseText != null) {
                    data = JSON.parse( this.responseText );
                    UpdateArduinoDisplay( data );
                }
            }
        }
    }
    request.open( "GET", "/~jje/site/api" + item, true );
    request.setRequestHeader("Content-Type",
            'application/json');
    request.send( );
}

function GetArduinoAll() {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (this.readyState == 4) {
            if (this.status == 200) {
                if (this.responseText != null) {
                    var data = JSON.parse( this.responseText );
                    UpdateArduinoDisplay( data );
                }
            }
        }
    }
    request.open( "GET", "/~jje/site/api", true );
    request.setRequestHeader("Content-Type",
            'application/json');
    request.send( );
}

function AlarmDayEnabled( al, sp ) {
    // al - alarm object
    // sp - span element

    var dy = al.dy;
    var dnm = sp.innerHTML;
    var state = 0;
    switch (dnm) {
        case 'Sunday':
            state = (dy & 0x01);
            break;
        case 'Monday':
            state = (dy & 0x02);
            break;
        case 'Tuesday':
            state = (dy & 0x04);
            break;
        case 'Wednesday':
            state = (dy & 0x08);
            break;
        case 'Thursday':
            state = (dy & 0x10);
            break;
        case 'Friday':
            state = (dy & 0x20);
            break;
        case 'Saturday':
            state = (dy & 0x40);
            break;
        default:
            state = 0;
    }
    if ( state > 0 ) {
        state = true;
    } else {
        state = false;
    }
    return state;
}

function DayNum2Name( num ) {
    var names = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    return names[num];
}

function FormatClockTime( c ) {
    var c_str;
    c_str = c.hr + ":" + c.mn + ":" + c.sc;
    return c_str;
}


window.onload = function() {
    addEventHandlers();
    GetArduinoAll();
}

// TODO LIST:
//      - set alarm control values to current alarm settings when
//        alarm index is changed
//      - add a dirty check before changing alarm control values
//        when alarm index is changed
//      - add LED display and controls


