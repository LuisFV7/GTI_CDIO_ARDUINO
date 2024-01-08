% Channel ID to read data from
readChannelID = 2396271;

% Field IDs for different parameters
TemperatureFieldID = 2;
LuminosityFieldID = 5;
SalinityFieldID = 3;
pHFieldID = 4;
HumidityFieldID = 1;

% Channel Read API Key
% If your channel is private, then enter the read API Key between the '' below:
readAPIKey = 'L9IPK6D3DKSEKXBU';

% Get data for the last 10 hours = 10 x 60 minutes.
tempF = thingSpeakRead(readChannelID, 'Fields', TemperatureFieldID, 'NumMinutes', 10 * 60, 'ReadKey', readAPIKey);
lumF = thingSpeakRead(readChannelID, 'Fields', LuminosityFieldID, 'NumMinutes', 10 * 60, 'ReadKey', readAPIKey);
salinityF = thingSpeakRead(readChannelID, 'Fields', SalinityFieldID, 'NumMinutes', 10 * 60, 'ReadKey', readAPIKey);
pHF = thingSpeakRead(readChannelID, 'Fields', pHFieldID, 'NumMinutes', 10 * 60, 'ReadKey', readAPIKey);
humidityF = thingSpeakRead(readChannelID, 'Fields', HumidityFieldID, 'NumMinutes', 10 * 60, 'ReadKey', readAPIKey);

% Create subplots for each parameter
subplot(3, 2, 1); % 3 rows, 2 columns, first subplot
histogram(tempF);
xlabel('Temperature (F)');
ylabel('Number of Measurements');
title('Histogram of Temperature Variation');

subplot(3, 2, 2); % 3 rows, 2 columns, second subplot
histogram(lumF);
xlabel('Luminosity (F)');
ylabel('Number of Measurements');
title('Histogram of Luminosity Variation');

subplot(3, 2, 3); % 3 rows, 2 columns, third subplot
histogram(salinityF);
xlabel('Salinity');
ylabel('Number of Measurements');
title('Histogram of Salinity Variation');

subplot(3, 2, 4); % 3 rows, 2 columns, fourth subplot
histogram(pHF);
xlabel('pH');
ylabel('Number of Measurements');
title('Histogram of pH Variation');

subplot(3, 2, 5); % 3 rows, 2 columns, fifth subplot
histogram(humidityF);
xlabel('Humidity');
ylabel('Number of Measurements');
title('Histogram of Humidity Variation');