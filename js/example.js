/*
	Example server that runs on http://your.domain/floodSensor/(int)isFlooded/(float)humidity/(float)temperature
*/
const BOT_TOKEN = "<your discord bot token here>";
const CHANNEL_ID = "<your discord channel id here>";
const PORT = 80;

const express = require('express');
const FloodSensor = require('./FloodSensor');
const fSensor = new FloodSensor(BOT_TOKEN, CHANNEL_ID);

const app = express();
app.get('/*', async (req, res) => {

	let status = 200;
	let out = '';

	// Fetch endpoint and args
	let args = req.path;
	if( args.charAt(0) === '/' )
		args = args.substr(1);

	args = args.split('/');
	const endpoint = args.shift();
	if( endpoint === 'floodSensor' ){
		try{
			out = await fSensor.run(...args);
		}catch(err){
			status = 400;
			out = err;
			console.log("Error", err);
		}
	}
	else
		status = 404;

	res.status(status);
	res.send(out);
	
});

app.listen(PORT, () => {
	console.log('Server online on port '+PORT)
});



