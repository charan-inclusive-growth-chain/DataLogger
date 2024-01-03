// Importing necessary modules
import express from 'express';
import axios from 'axios';

import { InfluxDBClient, Point } from '@influxdata/influxdb3-client';
import dotenv from 'dotenv';

// Load environment variables from .env file
dotenv.config();

// Create an instance of Express
const app = express();

// Using middleware to parse JSON in the request body
app.use(express.json());

const client = new InfluxDBClient({
    host: 'https://us-east-1-1.aws.cloud2.influxdata.com',
    token: process.env.TOKEN
  });


// Function to upload data to InfluxDB
async function uploadToInfluxDB(message) {
  

  const database = `datalogger`;

   // Extracting date, MCC, MNC, LAC, and Cellid from the message
   // Remove unnecessary characters from the message
  const cleanedMessage = message.replace(/[%$\r\n]/g, '');
  console.log(cleanedMessage)

  // Split the cleaned message by commas into a list
  const valueList = cleanedMessage.split(',');

  // Extract required values
  const phoneNumber = valueList[0].split('"')[1];
  const date = valueList[2].split('"')[1];
  const time= valueList[3].split('+')[0];
  const mcc = valueList[4].split(':')[1];
  const mnc = valueList[5].split(':')[1];
  const lac = parseInt(valueList[9].split(':')[1].toString(), 16); // Convert hex to decimal
  const cellid = parseInt(valueList[7].split(':')[1].toString(),16); // Convert hex to decimal

    const url = `https://cellphonetrackers.org/gsm/classes/Cell.Search.php?mcc=${mcc}&mnc=${mnc}&lac=${lac}&cid=${cellid}`;

    const response = await axios.get(url);

    // Parse the response to extract lat and lon
    const matchLatLon = response.data.match(/Lat=([\d.]+) Lon=([\d.]+)/);

    if (!matchLatLon) {
      throw new Error('Invalid response format');
    }

    const latitude = parseFloat(matchLatLon[1]);
    const longitude = parseFloat(matchLatLon[2]);

    console.log("Got location Details ", latitude, longitude);

    const point = Point.measurement('DataLoggerV3')
    .setTag('operator', 'loggerDetails')
    .setStringField('longitude', longitude.toString())
    .setStringField('latitude', latitude.toString())
    .setStringField('date', date.toString())
    .setStringField('date', time.toString())
    .setStringField('loggerId',phoneNumber)


  await client.write(point, database);

  client.close();
}

// Define a route for the '/upload' endpoint
app.post('/upload', (req, res) => {
  // Logging the JSON body from the request
  console.log('Received POST request at /upload with body:', req.body);

  // Extracting the relevant data from the request body
  const { message } = req.body;

  if (!message) {
    return res.status(400).json({ error: 'Missing message parameter' });
  }

  // Call the function to upload data to InfluxDB
  uploadToInfluxDB(message)
    .then(() => {
      // Sending a response to the client
      res.send('Data uploaded to InfluxDB successfully!');
    })
    .catch((error) => {
      console.error('Error uploading data to InfluxDB:', error);
      res.status(500).send('Internal Server Error');
    });
});


// Define a route for the '/query' endpoint
app.get('/query', async (req, res) => {
    console.log("query")
    const client = new InfluxDBClient({
        host: 'https://us-east-1-1.aws.cloud2.influxdata.com',
        token: process.env.TOKEN
      });
    
    try {
      // Construct the InfluxDB query
      const query = `SELECT * FROM 'DataLoggerV3' 
        WHERE time >= now() - interval '24 hours' AND 
        ('latitude' IS NOT NULL OR 'longitude' IS NOT NULL) order by time asc`

        const rows = await client.query(query, 'datalogger')
        console.log(rows);

        const result = []
        for await (const row of rows) {
            result.push(row);

           
            
        }
        console.log(result);
         res.status(200).json(result)
            } catch (error) {
            console.error('Error executing InfluxDB query:', error);
            res.status(500).send('Internal Server Error');
            }
        });

  

        // Set the server to listen on port 3000
        const port = 4000;
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
