/*
 * Chart and plotting functions.
 */


/*
 * Draws a line chart, provided with a parent element and a configuration object
 * containing data.  Returns chart object.
 */
function drawLineChart(el, config) {
    return c3.generate({
        bindto: el,
        data: {
            x: "#TIME", 
            columns: [
                ["#TIME", ...config.time],
                [config.label.split(" (")[0], ...config.data],
            ],
        },
        axis: {
            x: {
                type: "timeseries",
                tick: {
                    format: toTimeString,
                    count: 6,
                },
            },
            y: {
                label: config.label,
                tick: {
                    format: config.yTick || (x => Math.round(x)),
                },
            },
        },
        grid: {
            x: {
                show: true,
            },
            y: {
                show: true,
            },
        },
        legend: {
            show: false,
        },
        tooltip: {
            format: {
                value: config.yFormat || (x => {
                    let unit = config.label.split("(")[1].split(")")[0];
                    return Math.round(x) + " " + unit;
                }),
            },
        },
    });
}

/*
 * Add data to an existing chart.
 */
function addData(chart, time, data) {
    chart.flow({
        columns: [
            ["#TIME", ...time],
            [chart.data()[0].id, ...data],
        ],
        length: 0,
    });
}
