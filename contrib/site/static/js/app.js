/* Cthulhu Offerings website */
/* 2013-12-30 ZeeWolf */

Date.prototype.format = function(format) { //author: meizz
  var o = {
    "M+" : this.getMonth()+1, //month
    "d+" : this.getDate(),    //day
    "h+" : this.getHours(),   //hour
    "m+" : this.getMinutes(), //minute
    "s+" : this.getSeconds(), //second
    "q+" : Math.floor((this.getMonth()+3)/3),  //quarter
    "S" : this.getMilliseconds() //millisecond
  }

  if(/(y+)/.test(format)) format=format.replace(RegExp.$1,
    (this.getFullYear()+"").substr(4 - RegExp.$1.length));
  for(var k in o)if(new RegExp("("+ k +")").test(format))
    format = format.replace(RegExp.$1,
      RegExp.$1.length==1 ? o[k] :
        ("00"+ o[k]).substr((""+ o[k]).length));
  return format;
}

$(document).ready(function() {
	var diff = parseFloat($('#diff').text());
	var reward = parseFloat($('#reward').text());
	
	$('a[href^="#"]').click(function(){  
		var the_id = $(this).attr("href");  
		$('html, body').animate({  
			scrollTop:$(the_id).offset().top  
		}, 'slow');  
		return false;  
	});

	$('[data-time]').each(function() {
		var $this = $(this);
		var time = new Date($this.data('time') * 1000);
		$this.text(time.format('yyyy-MM-dd hh:mm:ss'));
	});
	
	switch($('body').attr('class').match(/[a-z]+/)[0]) {
		case 'index':

			if (localStorage && localStorage.hashrate) {
				$('#hashrate').val(!isNaN(localStorage.hashrate)?localStorage.hashrate:9);
			}
			
			$('#hashrate').on('keyup', function() {
				var hashrate = parseFloat($(this).val());
				var $offperday = $('#off_per_day');
				var $offperhour = $('#off_per_hour');
				var $btcperday = $('#btc_per_day');
				if (!isNaN(hashrate) && !isNaN(diff) && !isNaN(reward) && hashrate>0) {
					// calculate
					var timetoblock = ((diff * Math.pow(2,24)) / (hashrate*1000000)) / 3600;
					var blocksperday = 24 / timetoblock;
					var blocksperhour = blocksperday / 24;
					var offperhour = blocksperhour * reward;
					var offperday = blocksperday * reward;
					// display results
					$offperhour.text(offperhour.toFixed(1));
					$offperday.text(offperday.toFixed(1));
					if (off_data.price) {
						$btcperday.text((offperday * off_data.price).toFixed(8));
					}
				} else {
					// calculating impossible
					$offperhour.text('---');
					$offperday.text('---');
					$btcperday.text('---');
				}
				if (localStorage) {
					localStorage.hashrate = hashrate;
				}
				
			}).trigger('keyup');
			
		break;
		case 'ritual':
		break;
		case 'difficulty':
			Highcharts.theme = {
				colors: ["#DDDF0D", "#7798BF", "#55BF3B", "#DF5353", "#aaeeee", "#ff0066", "#eeaaee",
					"#55BF3B", "#DF5353", "#7798BF", "#aaeeee"],
				chart: {
					backgroundColor: {
						linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
						stops: [
							[0, 'rgb(96, 96, 96)'],
							[1, 'rgb(16, 16, 16)']
						]
					},
					borderWidth: 0,
					borderRadius: 0,
					plotBackgroundColor: null,
					plotShadow: false,
					plotBorderWidth: 0
				},
				title: {
					style: {
						color: '#FFF',
						font: '16px Lucida Grande, Lucida Sans Unicode, Verdana, Arial, Helvetica, sans-serif'
					}
				},
				subtitle: {
					style: {
						color: '#DDD',
						font: '12px Lucida Grande, Lucida Sans Unicode, Verdana, Arial, Helvetica, sans-serif'
					}
				},
				xAxis: {
					gridLineWidth: 0,
					lineColor: '#999',
					tickColor: '#999',
					labels: {
						style: {
							color: '#999',
							fontWeight: 'bold'
						}
					},
					title: {
						style: {
							color: '#AAA',
							font: 'bold 12px Lucida Grande, Lucida Sans Unicode, Verdana, Arial, Helvetica, sans-serif'
						}
					}
				},
				yAxis: {
					alternateGridColor: null,
					minorTickInterval: null,
					gridLineColor: 'rgba(255, 255, 255, .1)',
					minorGridLineColor: 'rgba(255,255,255,0.07)',
					lineWidth: 0,
					tickWidth: 0,
					labels: {
						style: {
							color: '#999',
							fontWeight: 'bold'
						}
					},
					title: {
						style: {
							color: '#AAA',
							font: 'bold 12px Lucida Grande, Lucida Sans Unicode, Verdana, Arial, Helvetica, sans-serif'
						}
					}
				},
				legend: {
					itemStyle: {
						color: '#CCC'
					},
					itemHoverStyle: {
						color: '#FFF'
					},
					itemHiddenStyle: {
						color: '#333'
					}
				},
				labels: {
					style: {
						color: '#CCC'
					}
				},
				tooltip: {
					backgroundColor: {
						linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
						stops: [
							[0, 'rgba(96, 96, 96, .8)'],
							[1, 'rgba(16, 16, 16, .8)']
						]
					},
					borderWidth: 0,
					style: {
						color: '#FFF'
					}
				},


				plotOptions: {
					series: {
						nullColor: '#444444'
					},
					line: {
						dataLabels: {
							color: '#CCC'
						},
						marker: {
							lineColor: '#333'
						}
					},
					spline: {
						marker: {
							lineColor: '#333'
						}
					},
					scatter: {
						marker: {
							lineColor: '#333'
						}
					},
					candlestick: {
						lineColor: 'white'
					}
				},

				toolbar: {
					itemStyle: {
						color: '#CCC'
					}
				},

				navigation: {
					buttonOptions: {
						symbolStroke: '#DDDDDD',
						hoverSymbolStroke: '#FFFFFF',
						theme: {
							fill: {
								linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
								stops: [
									[0.4, '#606060'],
									[0.6, '#333333']
								]
							},
							stroke: '#000000'
						}
					}
				},

				// scroll charts
				rangeSelector: {
					buttonTheme: {
						fill: {
							linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
							stops: [
								[0.4, '#888'],
								[0.6, '#555']
							]
						},
						stroke: '#000000',
						style: {
							color: '#CCC',
							fontWeight: 'bold'
						},
						states: {
							hover: {
								fill: {
									linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
									stops: [
										[0.4, '#BBB'],
										[0.6, '#888']
									]
								},
								stroke: '#000000',
								style: {
									color: 'white'
								}
							},
							select: {
								fill: {
									linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
									stops: [
										[0.1, '#000'],
										[0.3, '#333']
									]
								},
								stroke: '#000000',
								style: {
									color: 'yellow'
								}
							}
						}
					},
					inputStyle: {
						backgroundColor: '#333',
						color: 'silver'
					},
					labelStyle: {
						color: 'silver'
					}
				},

				navigator: {
					handles: {
						backgroundColor: '#666',
						borderColor: '#AAA'
					},
					outlineColor: '#CCC',
					maskFill: 'rgba(16, 16, 16, 0.5)',
					series: {
						color: '#7798BF',
						lineColor: '#A6C7ED'
					}
				},

				scrollbar: {
					barBackgroundColor: {
							linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
							stops: [
								[0.4, '#888'],
								[0.6, '#555']
							]
						},
					barBorderColor: '#CCC',
					buttonArrowColor: '#CCC',
					buttonBackgroundColor: {
							linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
							stops: [
								[0.4, '#888'],
								[0.6, '#555']
							]
						},
					buttonBorderColor: '#CCC',
					rifleColor: '#FFF',
					trackBackgroundColor: {
						linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
						stops: [
							[0, '#000'],
							[1, '#333']
						]
					},
					trackBorderColor: '#666'
				}
			};

			// Apply the theme
			var highchartsOptions = Highcharts.setOptions(Highcharts.theme);	
				
			$.getJSON('/diff.json', function(data) {
				data.length ? $('#diff-chart').highcharts('StockChart', {
					chart: {
						backgroundColor: 'transparent',
						borderWidth: 0,
						borderColor: '#333333',
						borderRadius: 0,
						plotBackgroundColor: 'transparent',
						plotShadow: false,
						shadow: false
					},
					rangeSelector: {
						buttons: [{
							type: 'day',
							count: 3,
							text: '3d'
						}, {
							type: 'week',
							count: 1,
							text: '1w'
						}, {
							type: 'month',
							count: 1,
							text: '1m'
						}, {
							type: 'month',
							count: 6,
							text: '6m'
						}, {
							type: 'all',
							text: 'All'
						}],
						selected: 0
					},			
					credits: {
						enabled: false
					},
					legend: {
						enabled: false
					},
					plotOptions: {
						line: {
							color: '#dda149',
							marker: {
								enabled: false
							},
							states: {
								hover: {
									lineWidth: 2
								}
							},
							turboThreshold: 0
						}
					},
					title: null,
					xAxis: {
						type: 'datetime',
						gridLineWidth: 0,
						minorGridLineWidth: 0,
						lineColor: '#333333',
						tickColor: '#333333',
						labels: {
							formatter: function() {
								return Highcharts.dateFormat('%d %b', this.value);
							},
							rotation: -90
						}
						
						//gridLineColor: '#333333',
						//minorGridLineColor: '#333333',
						//minorTickColor: '#333333',
					},
					yAxis: {
						gridLineWidth: 0,
						minorGridLineWidth: 0,
						lineColor: '#333333',
						lineWidth: 1,
						tickColor: '#333333',
						//gridLineColor: '#333333',
						//minorGridLineColor: '#333333',
						//minorTickColor: '#333333',
						tickWidth: 1,
						title: {
							text: 'Diff'
						},
						min: 0
					},
					tooltip: {
						shared: false,
						formatter: function() {
							var tip;
							tip = new Date(this.x).format('yyyy-MM-dd hh:mm:ss') + '<br />' +
										'<span style="font-weight:bold;color:#dda149">'+ this.point.series.name +':</span> <span style="font-size:18px;font-weight:bold;color:#dda149">'+ this.y +'</span>';
							if (this.point.height) {
								tip += '<br /><b>Height: </b> ' + this.point.height;
							}
							return tip;
						}
					},			
					series: [{
						name: 'Diff',
						data: data,
						id : 'off'
					}/*, {
						type : 'flags',
						data : [{
							x : Date.UTC(2014, 1, 1),
							title : 'H',
							text : 'First halving 5 -> 2.5 OFF'
						}],
						onSeries : 'off',
						shape : 'circlepin',
						width : 16
					}*/]
				}) : $('#diff-chart').text('No data available Try later.');
			}).fail(function() {
				$('#diff-chart').text('No data available Try later.');
			});
		break;
	}
});
