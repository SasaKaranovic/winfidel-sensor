var updateIntervalID;

$(document).ready(function(){
    refresh_reading();
    get_calibration();
    start_periodic_update();
    $('[data-toggle="tooltip"]').tooltip()


    $("#btn-calibrate").on( "click", function() {
        const cal_adc = $('#input-adc-last').val();
        const cal_mm = $('#input-cal-diameter').val();

        if(!isNaN(parseFloat(cal_mm)))
        {
            $('#btn-calibrate').text('Please wait...');
            create_calibration_point(cal_adc, cal_mm);
        }
        else
        {
            $("#invalid-value").show();
        }
    });


    $("#adc-label").on("dblclick", function() {


        let manual = $("#input-adc-last").prop('disabled');
        if (manual == true)
        {
            alert("Enabling manual calibration!");
            $("#input-adc-last").prop('disabled', false);
            $("#input-adc-last").tooltip('disable');
            stop_periodic_update();
            $('#input-adc-last').val('');
            $('#input-adc-last').focus();
        }
        else
        {
            $("#input-adc-last").prop('disabled', true);
            $("#input-adc-last").tooltip('enable');
            start_periodic_update();
        }
    });

});

function get_calibration()
{
    $.getJSON( '/api/calibration/read', function( data ) {

        const total_points = data['data'].length;
        $('#calibration-points-count').text(total_points);
        $("#calibration-points > tbody").html("");

        for (let i = 0; i < total_points; i++) {
            if(i==0 || i== total_points-1)
            {
                $('table#calibration-points').append('<tr class="d-flex"><td class="col-5">'+ data['data'][i][0] +'</td><td class="col-5">'+ data['data'][i][1] +' mm</td><td class="col-2"></td></tr>');
            }
            else
            {
                $('table#calibration-points').append('<tr class="d-flex"><td class="col-5">'+ data['data'][i][0] +'</td><td class="col-5">'+ data['data'][i][1] +' mm</td><td class="col-2"><a href="javascript:remove_calibration_point('+ data['data'][i][1] +');">Remove</a></td></tr>');
            }
        }

    });
}

function create_calibration_point(adc, mm)
{
    $.get( '/api/calibration/create?adc='+ adc +'&mm='+ mm, function( data ) {
        if (data['status'] == 'ok')
        {
            location.reload();
        }
    });
}

function remove_calibration_point(diameter)
{
    $.get( '/api/calibration/remove?mm='+ diameter, function( data ) {
        console.log(data);

        if (data['status'] == 'ok')
        {
            location.reload();
        }
    });
}

function refresh_reading()
{
    $.getJSON( '/api/diameter/read', function( data ) {
        $('#input-adc-last').val(data['data']['adc']);
    });
}

function reset_stats()
{
    $.get( '/api/diameter/reset');
}


function start_periodic_update()
{
    updateIntervalID = window.setInterval(function(){
      refresh_reading();
    }, 1000);

}

function stop_periodic_update()
{
    clearInterval(updateIntervalID);
}

