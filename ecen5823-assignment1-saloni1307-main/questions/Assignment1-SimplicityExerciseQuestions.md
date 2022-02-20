Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**
   Answer: 5.53mA


**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**
   Answer: 5.57mA


**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, 
referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate. Extra credit is avilable for this question and depends on your answer.**
   Answer: There is no significant difference between the two answers. In both modes, the system draws same amount of current. This behavior is what we would expect since its the same functionality. However, the significance of these modes and current ratings specified lies in the safe and reliable working of the board. According to the Blue Gecko Board specifications; Strong drive strength can drive upto 10mA of current for a given port whereas Weak drive strength can drive upto 1mA. This means that even though in Weak mode the LED port should drive maximum 1mA, it is forced to drive more current in order to use the LEDs. Since, the port is driving more current than it is rated for, it is highly possible that eventually the part will heat up and then get destroyed. This concept can be explained in detail using an analogy: Suppose John Doe can consume only two apples and we keep on feeding him more and more apples until he explodes. Similarly, when we supply current to any device, it will keep on driving current until it is fatally destroyed. In our case, this phenomenon can damage the LEDs, or the port or complete board.  


**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: 5.12mA


**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: 5.45mA


