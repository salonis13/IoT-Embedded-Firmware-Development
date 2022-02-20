Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period?
   Answer: 9.78 uA
   <br>Screenshot:  
   ![Avg_current_per_period](screens<img width="556" alt="avg_current_per_period" src="https://user-images.githubusercontent.com/77704335/134750800-3efee66f-ab7d-4762-b862-ea1803de6e5f.png">
hots/assignment4/avg_current_per_period.jpg)  

2. What is the average current when the Si7021 is Powered Off?
   Answer: 2.73 uA
   <br>Screenshot:  <img width="556" alt="avg_current_LPM_off" src="https://user-images.githubusercontent.com/77704335/134750812-622fc6cb-c207-4146-95fd-7f759db9c534.png">

   ![Avg_current_LPM_Off](screenshots/assignment4/avg_current_lpm_off.jpg)  

3. What is the average current when the Si7021 is Powered On?
   Answer: 360 uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](screenshots/assignmen<img width="557" alt="avg_current_LPM_on" src="https://user-images.githubusercontent.com/77704335/134750823-da30b3eb-a458-467f-8437-041a3c790d75.png">
t4/avg_current_lpm_on.jpg)  

4. How long is the Si7021 Powered On for 1 temperature reading?
   Answer: 96 ms
   <br>Screenshot:  <img width="557" alt="duration_LPM_on" src="https://user-images.githubusercontent.com/77704335/134750844-85896b50-1243-4665-b8cf-30f892f4c9d6.png">

   ![duration_lpm_on](screenshots/assignment4/avg_current_lpm_on.jpg)  

5. Compute what the total operating time of your design for assignment 4 would be in hours, assuming a 1000mAh battery power supply?
   Answer: Total operating time of the design = Battery capaity (in mAh) / Current drawn (in mA)
                                              = 1000/11.83
                                              = 84.5 hours
   
6. How has the power consumption performance of your design changed since the previous assignment?
   Answer: For previous assignment average current consumed by the system when it is on was 180uA. Hence, if we consider the same 1000mAh battery, the total operating time will only be 4.5 hours which is 17x less than what we got in this assignment. In previous assignment, as we implemented blocking code instead of interrupt driven design, the processor was always on when the sensor was on. During delays and I2C read-write operations also, the processor was consuming more power which was not required. However, in this assignment, the processor is sleeping when it is not doing any operations which drastically reduces power consumption for same operation. 
   
7. Describe how you have tested your code to ensure you are sleeping in EM1 mode during I2C transfers.
   Answer: To test for EM1 mode, I leveraged the output of energy profiler as well as the ability of I2C transfer to be carried out only in EM0 or EM1 mode.
   - Firstly, if we observe the energy profile closely, every instant when we are adding EM1 requirement in our code (after COMP1 events), there is a small dip in the energy consumption. The current measurement at this point is about 4mA which is expected in EM1 mode. And after completing the transfer, processor enters EM3 mode. Here is a screenshot of the energy profiler explaining the behaviour. 
  
<img width="554" alt="em1_transfer" src="https://user-images.githubusercontent.com/77704335/134751458-4efb8e43-cdb4-43d0-b954-822c13a7a34d.png">

- Next, I tried running the code in EM3 mode without adding EM1 requirement. In this case, as expected, the read-write transfer operation was not done on I2C as it cannot operate in energy mode less than EM1. After it receives write command, it again goes to sleep in EM3 mode and hence the transfer does not take place. 
- Next, I tried adding EM2 mode requirement in code instead of EM1 and again as expected no I2C transfer took place.
- Next, I tried running the code in EM2 mode without adding any EM1 requirement and here, no I2C transfer took place. 
- Next, I tred running the processor in EM2 mode and also added EM1 mode requirement wherever necessary. Here, all I2C transfers took place successfully. Temperature measurements were taken correctly.
      Hence, from all these observations, I can safely say that the processor successfully entered EM1 mode during I2C transfers. 
