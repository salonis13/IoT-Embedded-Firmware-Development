Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.*

1. Provide screen shot verifying the Advertising period matches the values required for the assignment.
   <br>Screenshot:  
   <img width="560" alt="advertising_period" src="https://user-images.githubusercontent.com/77704335/135959031-52a42fdf-5f95-43fc-bc78-229f75753ac8.png">

   ![advertising_period](screenshots/assignment5/advertising_period.png)  

2. What is the average current between advertisements?
   Answer: 1.89 uA
   <br>Screenshot:  
   <img width="559" alt="avg_current_between_advertisements" src="https://user-images.githubusercontent.com/77704335/135959092-191e7a0d-ffcd-46b5-a8e8-cdb829493700.png">

   ![avg_current_between_advertisements](screenshots/assignment5/avg_current_between_advertisements.png)  

3. What is the peak current of an advertisement? 
   Answer: 26.98 mA
   <br>Screenshot:  
   <img width="559" alt="peak_current_of_advertisement" src="https://user-images.githubusercontent.com/77704335/135959122-00c8c9f1-79f8-4f60-b113-665e2a7555c9.png">

   ![peak_current_of_advertisement](screenshots/assignment5/peak_current_of_advertisement.png)  

4. Provide screen shot showing the connection interval settings. Do they match the values you set in your slave(server) code or the master's(client) values?.
   The connection interval observed is 300ms. This value does not match our value entered in code i.e. 75ms due to slave latency value 3.
   <br>Screenshot: 
   <img width="559" alt="connection_interval" src="https://user-images.githubusercontent.com/77704335/135959170-8f84ce82-ab91-4c78-b38a-7b90b69e9172.png">

   ![connection_interval](screenshots/assignment5/connection_interval.png)  

5. What is the average current between connection intervals?
   Answer: 2.13 uA
   <br>Screenshot:  
   <img width="559" alt="avg_current_between_connection_intervals" src="https://user-images.githubusercontent.com/77704335/135959351-2fc78427-6992-4a06-9988-2768550c6e4c.png">

   ![avg_current_between_connection_intervals](screenshots/assignment5/avg_current_between_connection_intervals.png)  

6. If possible, provide screen shot verifying the slave latency matches what was reported when you logged the values from event = gecko_evt_le_connection_parameters_id. 
   Here we can observe that the parameters set by the code are as follows:
   
   <img width="517" alt="parameters" src="https://user-images.githubusercontent.com/77704335/135962163-fb5dcac7-a606-41b4-bbc2-8739364adbd6.png">

   Based on these values we can plot the below timing diagram for connection interval, connection event and slave latency
   
   ![image](https://user-images.githubusercontent.com/77704335/135963181-59f21f83-201d-4a4d-8edb-c0fe6e253652.png)
   
   Since, the slave latency is 3, the slave can skip upto 3 connections and hence we can get the desired slave off the air timing of about 300ms. 
   <br>Screenshot:  
   
   <img width="559" alt="slave_latency" src="https://user-images.githubusercontent.com/77704335/135959369-363cd6d9-be27-4f20-8e97-d8370f73be4b.png">

   ![slave_latency](screenshots/assignment5/slave_latency.png)  

7. What is the peak current of a data transmission when the phone is connected and placed next to the Blue Gecko? 
   Answer: 22.57 mA
   <br>Screenshot:  <img width="557" alt="peak_current_phone_next_to" src="https://user-images.githubusercontent.com/77704335/135960974-e1b48cf6-ec02-4806-a318-513aa83ac672.png">

   ![peak_current_phone_next_to](screenshots/assignment5/peak_current_phone_next_to.png)  
   
8. What is the peak current of a data transmission when the phone is connected and placed approximately 20 feet away from the Blue Gecko? 
   Answer: 21.77 mA
   
   <img width="557" alt="peak_current_phone_20ft_away" src="https://user-images.githubusercontent.com/77704335/135960995-c6d1308a-a2c7-409c-86e0-eb192be4b5c5.png">

   <br>Screenshot:  
   ![peak_current_phone_20ft_away](screenshots/assignment5/peak_current_phone_20ft_away.png)  
   
