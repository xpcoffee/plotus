BareMinimumPlotter
====================
Emerick Bosch || 
July 2014

NOTE: This is still a work in progress and is not yet ready for distribution of any kind.

----

##About BareMinimumPlotter
Solutions to technical problems, such as engineering design problems, are very often non-singular; they can be more accurately represented as a region or 'space' of solutions rather than a single point. 

While this method of solving problems is not novel, it is certainly under-represented and sparsely used. Practically, the near totality of calculation software and problem solving techniques in use today focus on attaining a single, optimized solution, rather than these regions.
Arriving at an acceptable solution is often heavily dependent on sound the contextual knowledge and experience of the person doing the calculation. 

BareMinimumPlotter attempts to provide a different tool for problem solving, by allowing the user to combine and then plot inequalities for a range of parameters. The result is a visual representation of solution-space, from which informed decisions can be made, even with limited amounts of experience.

It is hoped that BareMinimumPlotter provides a tool with which the user can rapidly set up and "mess with" a technical problem to quickly understand it and arrive at an adequate solution.

----

##Quickstart Guide
###0. Cases
The goal of BareMimimumPlotter is to visually represent problem senarios. Because of this, a completely configured and plotted set of inequalities is reffered to as a 'case'. Once cases have been created, they can be saved opened and compared to other cases using the steps below.

###1. Inequalities
Enter inequalities; to add another inequality, press the "Add Inequality" button.

Specify plotting conditions, such as colour and marker shape, by using the drop-down boxes.

Select the interaction between inequalities (Intersection, Union, Subtraction). Note that if Subtraction is selected, the following inequality region will be subtracted from the current one.

###2. Variables/Parameters
Enter variables/parameters; to add another variable/parameter, press the "Add Variable" button

Two variables must be chosen to be plotted along the horizontal and vertical axes. The 'Min' and 'Max' values of these variables determine the limits of the plot. The 'Steps' values determine the grid along which the inequalities are evaluated and plotted.

Other variables will act as constants - the value of the constants can be selected using the slider.

###3.Plotting
Press the "Plot" button to plot the current case. 

A plot will only be created if all inequalities and variables are fully defined and valid.

If the plot is successful, the case is considered complete, and can then be saved for future use.

###4.Saving a Case
Once happy with plot of a case, it can be saved by clicking

    File > Save As...
    
Data from the plot, as well as all input data (inequalities, variables, etc) are then saved as a JSON file.

###5.Opening a Previously Saved Case
To open a previously saved case, click

    File > Open
    
A file explorer will open. Navigate to your JSON save file, select it and click [Open] 

----

##Tips and Tricks
###Speed
Evaluation and plotting speed is influenced by three main factors:
- the amount of 'Steps' used for vertical and horizontal axis variables
- the complexity of the inequalities
- the amount of inequalities

In order to reduce evaluation time, it is suggested that the 'Steps' be kept as low as possible. Should a more accurate plot be needed, consider cropping the 'Min' and 'Max' values of the vertical and horizontal axis variables to focus on the region of interest.

###Aliasing
Step values being too low may lead to aliasing, where regions of a solution become lost or noisy. When unsure about aliasing, it is advisably to increase the step values and crop the plot to concentrate on problem-areas.

###Inconsistencies
Certain inequalities will contain mathematical inconsistencies, such as assymptotes. It is important to keep the possibility of inconsitencies in mind, as it is possible to miss them under certain conditions. E.g.

    x/y < y
    
Has a discontinuity at [y = 0]. This will be apparent if [x] and [y] are defined as follows:

    x,  min:0,    max:2,  steps: 50
    y,  min: -2,  max:2,  steps: 51
    
However the discontinuity is lost if [y] is defined with:

    steps:50
