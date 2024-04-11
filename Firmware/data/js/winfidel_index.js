let measurements_chart;

$(function() {
	// Take new reading
	// refresh_reading();

    var intervalId = window.setInterval(function(){
      refresh_reading();
    }, 1000);

  const ctx = document.getElementById('myChart');

  measurements_chart = new Chart(ctx, {
    type: 'line',
      data: {
        datasets: [{
          data: []
        }]
      },
    options: {
      scales: {
        y: {
            beginAtZero: false,
            suggestedMin: 1.35,
            suggestedMax: 2.25,
            ticks: {
              // forces step size to be 50 units
              stepSize: 0.05
            }
        },
        x: {
            beginAtZero: false,
            reverse: true
        }
      },
      plugins: {
          legend: {
            display: false,
          }
        }
    }
  });

});


function chart_add_point(newData) {
    measurements_chart.data.datasets.forEach((dataset) => {
        // dataset.data.push(newData);
        dataset.data.unshift(newData);
    });

    // measurements_chart.update();
    measurements_chart.update('none');
}


function refresh_reading()
{
    $.getJSON( '/api/diameter/read', function( data ) {

        $('#reading-last').text(data['data']['diameter']);
        $('#reading-min').text(data['data']['min']);
        $('#reading-avg').text(data['data']['avg']);
        $('#reading-max').text(data['data']['max']);
        $('#reading-count').text(data['data']['count']);

        chart_add_point({x: get_timestamp(), y: data['data']['diameter']});
    });
}

function reset_stats()
{
    $.get( '/api/diameter/reset');
}

function get_timestamp()
{
    let date = new Date();
    return twoDigitPad(date.getHours()) +':'+ twoDigitPad(date.getMinutes()) +':'+ twoDigitPad(date.getSeconds());
}

function twoDigitPad(num) {
    return num < 10 ? "0" + num : num;
}
