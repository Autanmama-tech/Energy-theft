% ThingSpeak channel details
channelID = 3040106;
readAPIKey = 'TGDC368XI10GCZSC';
fieldID = 2;  % Power usage is in field 2

% Read last 20000 points from ThingSpeak
[data, time] = thingSpeakRead(channelID, 'Fields', fieldID, ...
                              'ReadKey', readAPIKey, ...
                              'NumPoints', 8000);

if isempty(data)
    error('No data retrieved from ThingSpeak');
end

% Remove NaN values
validIdx = ~isnan(data);
data = data(validIdx);
time = time(validIdx);

% Convert time to datetime (ensure correct format)
time = datetime(time,'InputFormat','yyyy-MM-dd''T''HH:mm:ss''Z');

% Compute time differences in hours
dt = hours(diff(time));

% Compute energy segments (Wh)
energySegments = data(1:end-1) .* dt;

% Total energy consumption (Wh)
totalEnergy = sum(energySegments);

% Billing rate
rate = 100; % Naira per Wh
billAmount = totalEnergy * rate;

%% Generate PDF Bill
import mlreportgen.report.*
import mlreportgen.dom.*

rpt = Report('Electricity_Bill','pdf');
open(rpt);

% Title Page
tp = TitlePage;
tp.Title = 'Electricity Consumption Bill';
tp.Subtitle = sprintf('Billing Period: %s to %s', ...
    datestr(min(time)), datestr(max(time)));
tp.Author = 'Automated Billing System';
tp.Publisher = 'Smart Metering Service';
add(rpt,tp);

% Table of Contents
toc = TableOfContents;
add(rpt,toc);

% Billing Chapter
ch1 = Chapter('Title','Billing Summary');

p1 = Paragraph(sprintf(['This report summarizes the electricity usage ' ...
    'for the period %s to %s. Data was obtained from ThingSpeak ' ...
    '(Channel ID: %d). Energy usage was calculated as Power × Time. ' ...
    'The billing rate is ₦%d per Watt-hour (Wh).'], ...
    datestr(min(time)), datestr(max(time)), channelID, rate));
add(ch1,p1);

% Energy usage
p2 = Paragraph(sprintf('Total Energy Consumed: %.2f Wh', totalEnergy));
p2.Bold = true; p2.Color = 'red';
add(ch1,p2);

% Bill amount
p3 = Paragraph(sprintf('Total Bill: ₦%.2f', billAmount));
p3.Bold = true; p3.Color = 'blue';
add(ch1,p3);

% Detailed Table
tbl = FormalTable(...
    {'Date-Time','Power (W)','Δt (hr)','Energy (Wh)'}, ...
    [cellstr(datestr(time(1:end-1))) num2cell(data(1:end-1)) num2cell(dt) num2cell(energySegments)]);
tbl.Header.TableEntriesHAlign = 'center';
tbl.Width = '100%'; tbl.Border = 'solid'; tbl.ColSep = 'solid'; tbl.RowSep = 'solid';
add(ch1,tbl);

add(rpt,ch1);

close(rpt);
rptview(rpt);