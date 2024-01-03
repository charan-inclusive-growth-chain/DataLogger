import {InfluxDBClient, Point} from '@influxdata/influxdb3-client'

// ECMAScript Modules syntax
import dotenv from 'dotenv';
dotenv.config();

const token = process.env.INFLUXDB_TOKEN

async function main() {
    const client = new InfluxDBClient({host: 'https://us-east-1-1.aws.cloud2.influxdata.com', token: process.env.TOKEN})

    // following code goes here

    let database = `datalogger`

    const points =
        [
            Point.measurement("census")
                .setTag("location", "Klamath")
                .setIntegerField("bees", 23),
            Point.measurement("census")
                .setTag("location", "Portland")
                .setIntegerField("ants", 30),
            Point.measurement("census")
                .setTag("location", "Klamath")
                .setIntegerField("bees", 28),
            Point.measurement("census")
                .setTag("location", "Portland")
                .setIntegerField("ants", 32),
            Point.measurement("census")
                .setTag("location", "Klamath")
                .setIntegerField("bees", 29),
            Point.measurement("census")
                .setTag("location", "Portland")
                .setIntegerField("ants", 40)
        ];

    for (let i = 0; i < points.length; i++) {
        const point = points[i];
        await client.write(point, database)
            // separate points by 1 second
            .then(() => new Promise(resolve => setTimeout(resolve, 1000)));
    }


    client.close()
}

main()