An FPS level generator made for my honours project. The paper explaining what the thought process behind the algorithm is kept here: 

This algorithm is designed to generate map layouts for arena style FPS maps for team vs team pvp gameplay (Think halo gameplay with red vs blue). The algorithm determines room size and placement and also effective spawn point placement in the level.

The closer the rooms are to the center the less spawn points and the bigger the generated room is, the map is split into 2 different zones for each different team with spawn points being placed more frequently the closer the room is to the team's base.

The algorithm outputs a json file of the level placement which can be parsed into another game to be generated into a playable level.

One extra feature that the paper goes over is the placement of mid sized cover in the middle of the rooms. The spawn placement is determined through a heatmap generated of the level simulated what players can see when walking into a room. This heatmap would be reused for the placement of cover and weapon drops, I'll go back and add it in at some point.

Uses SFML for displaying the level graphically and rapidjson for outputting the level files. The build output folders in x64/ have necessary .dll files the resultant .exe files need to run.

Press h to toggle the heatmap overlay for the level output display.