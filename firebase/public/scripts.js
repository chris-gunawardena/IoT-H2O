var charts_loaded = false;
var data_loaded = false;
var chart_data = null;
var days = -8;
var readings_date = new Date();

var draw_chart = () => {
  if (!charts_loaded || !chart_data) return;

  var data = new google.visualization.DataTable();
  data.addColumn("timeofday", "Time of Day");
  data.addColumn("number", "Water level");
  data.addRows(chart_data);
  var chart = new google.charts.Bar(document.getElementById("chart_div"));
  chart.draw(
    data,
    google.charts.Bar.convertOptions({
      title: "Daily water intake",
      height: 450
    })
  );
}

var get_start_end_for = date => [date.setHours(0, 0, 0, 0), date.setHours(23, 59, 59, 999)];

var  load_data = () => {
  readings_date.setDate(new Date().getDate() + days);
  $('#date').text(readings_date.toString().substr(0,15));
  var [start, end] = get_start_end_for(readings_date);
  readings_ref
    .orderByChild("timestamp")
    .startAt(start)
    .endAt(end)
    .once("value")
    .then(snapshot => {
      chart_data = [];
      snapshot.forEach(reading => {
        let value = reading.val();
        let date = new Date(value.timestamp);
        chart_data.push([
          [date.getHours(), date.getMinutes(), date.getSeconds()],
          value.level
        ]);
      });
      data_loaded = true;
      draw_chart();
    });
}


firebase.initializeApp({
  apiKey: "AIzaSyAbNP6VWt2u3Pozwc6L-BTK_uasDhlsJYc",
  authDomain: "water-9dbfa.firebaseapp.com",
  databaseURL: "https://water-9dbfa.firebaseio.com"
});
var readings_ref = firebase.database().ref("/users/chris/readings");
load_data();

$('.pager li').on('click', (e) => {
  days = days + $(e.target).data('days');
  load_data();
});

$(window).resize(function(){
    draw_chart();
});

google.charts.load("current", { packages: ["bar"] });
google.charts.setOnLoadCallback(() => {
  charts_loaded = true;
  draw_chart();
});

