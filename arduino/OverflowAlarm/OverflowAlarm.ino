/*
	Todo: Rebind the pullup on sensor to an IO pin and enable it when reading
*/
#include <WiFiManager.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266HTTPClient.h>

DHTesp dht;

const uint8_t PIN_WATER_SENSE = A0;
const uint8_t PIN_CFG = D1;
const uint8_t PIN_DHT = D2;
const uint8_t PIN_LED_R = D5;			// Shows the flood status, blinks twice on boot.
const uint8_t PIN_LED_B = D6;			// Network status.
										// Solid blue = in config mode
										// 10 rapid blinks = failed to send, but connected
										// 1 rapid blink = successfully sent
const uint8_t PIN_WATER_SENSE_EN = D7;	// Enables water sensor

const uint16_t WATER_THRESH = 200;		// When ADC is above this value, consider it flooded
const char *URL = "<your_url>";

bool flooded = false;					// If the flood sensor has detected something
float temperature = 0;					// Celsius
float humidity = 0;						// Percent

uint32_t last_tick = 0;					// Last send to internet
uint32_t last_check = 0;				// Last flood check
const uint32_t TICK_RATE = 30e3;		// 30 sec between each read
HTTPClient http;
WiFiClient client;

void blink( uint8_t pin, uint8_t times, uint8_t rate = 100 ){

	for( uint8_t i = 0; i < times; ++i ){
		
		digitalWrite(pin, HIGH);
		delay(rate/2);
		digitalWrite(pin, LOW);
		delay(rate/2);

	}

}

bool sendData(){
	// Todo, send data to URL

	char out[128];
	strcpy(out, URL);
	strcat(out, flooded ? "1/" : "0/");
	char flt[10];
	dtostrf(humidity, 0, 1, flt);
	strcat(out, flt);
	strcat(out, "/");
	dtostrf(temperature, 0, 1, flt);
	strcat(out, flt);

	Serial.printf("Attempting data send to: %s\n", out);
	Serial.println(WiFi.status());


	http.begin(client, out);
	
	// Send HTTP GET request
	int httpResponseCode = http.GET();
	bool success = httpResponseCode > 0;
	if( success ){
		Serial.print("HTTP Response code: ");
		Serial.println(httpResponseCode);
		String payload = http.getString();
		Serial.println(payload);
		blink(PIN_LED_B, 1, 100);
	}
	else {
		Serial.print("Error code: ");
		Serial.println(httpResponseCode);
		blink(PIN_LED_B, 10, 100);
	}
	// Free resources
	http.end();

	return success;
}

// 

void setup(){

	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	pinMode(PIN_WATER_SENSE_EN, OUTPUT);
	digitalWrite(PIN_WATER_SENSE_EN, LOW);
	pinMode(PIN_WATER_SENSE, INPUT);
	pinMode(PIN_CFG, INPUT_PULLUP);
	
	pinMode(PIN_LED_R, OUTPUT);
	pinMode(PIN_LED_B, OUTPUT);
	
	dht.setup(PIN_DHT, DHTesp::DHT22);
	delay(dht.getMinimumSamplingPeriod());

	Serial.println("IT BEGINS!!");

	blink(PIN_LED_R, 3, 250);
	
	WiFiManager wifiManager;
	digitalWrite(PIN_LED_B, HIGH);
	if( !digitalRead(PIN_CFG) ){
		Serial.println("Forcing config portal");
		wifiManager.startConfigPortal("Flood");
	}
	else
		wifiManager.autoConnect("Flood");
	digitalWrite(PIN_LED_B, LOW);


}

void loop(){

	const uint32_t ms = millis();
	

	if( ms-last_tick > TICK_RATE || !last_tick ){
		last_tick = ms;

		temperature = dht.getTemperature();
		humidity = dht.getHumidity();

		sendData();

	}

	if( ms-last_check > 1000 ){

		digitalWrite(PIN_WATER_SENSE_EN, HIGH);
		delay(100);
		uint16_t reading = analogRead(PIN_WATER_SENSE);
		flooded = reading > WATER_THRESH;
		digitalWrite(PIN_WATER_SENSE_EN, LOW);
		digitalWrite(PIN_LED_R, flooded);
		//Serial.println(reading);

	}

}

