const { Client, GatewayIntentBits, ActivityType } = require("discord.js");
const client = new Client({
  	intents: [GatewayIntentBits.Guilds, GatewayIntentBits.GuildMessages, GatewayIntentBits.GuildMessageReactions]
});

client.on("ready", () => {
	console.log("Logged in as ", client.user.tag);
});
/*
client.on("messageCreate", msg => {
	console.log("Got message ", msg);
});
*/

module.exports = class FloodSensor{

	constructor( botkey, chan ){

		this.channel = chan;
		this.temperature = 0;
		this.humidity = 0;
		this.flooded = false;
		this.last_message = 0;
		this.timeout_interval = null;
		this.online = true;
		this.PING_TIMEOUT = 3600e3;		// How long before the bot should say it's lost connection
		this.botkey = botkey;

		client.login(botkey);

	}

	getChannel(){
		return client.channels.cache.get(this.channel);
	}

	async run( flooded = false, humidity = 0, temperature = 0 ){

		flooded = Boolean(Math.trunc(flooded));
		humidity = Math.trunc(humidity) || 0;
		temperature = +Math.round(temperature*10)/10;

		if( flooded && !this.flooded && Date.now()-this.last_message > 600e3 ){
			this.last_message = Date.now();
			let message = 'VARNING! Översvämning har upptäckts i lokalen!';
			this.getChannel().send(message);
		}

		this.temperature = temperature;
		this.humidity = humidity;
		this.flooded = flooded;

		console.log("Flooded", flooded, "Temp ", temperature, "hum", humidity);
		let activity = "🌡️"+temperature+"C ☁️"+humidity+"%";
		if( this.flooded ){
			activity = "⚠️ FLOODED ⚠️ - "+activity;
		}
		client.user.setActivity(activity, {
			type : ActivityType.Watching
		});

		if( !this.online ){
			this.getChannel().send("Sensor online!");
		}

		this.online = true;
		clearInterval(this.timeout_interval);
		this.timeout_interval = setInterval(() => {
			
			if( this.online ){
				this.getChannel().send("Ingen data har mottagits från sensorn på ett tag... har något hänt med anslutningen?");
			}
			this.online = false;
			
		}, this.PING_TIMEOUT);


		return "ok";

	}
	

}


