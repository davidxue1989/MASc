import csv
import collections #for ordereddict
import matplotlib.pyplot as plt
import numpy as np


# steps:
# 0	no step
# 1	intro
# 2	turn on water
# 3	soap
# 4	scrub
# 5	rinse
# 6	turn off water
# 7	dry
# 8	all done
# 9	wet

# P_R_PandR:
# 0	no prompt
# 1	P
# 2	R
# 3	P+R

# P involvement:
# 0	P behind doors
# 1	P prompts for compliance
# 2	P prompts for step
# 3	P behind C
# 4	P alone

# P verbal:
# 0	no prompt
# 1	prompt for compliance
# 2	prompt for step

# P gesture:
# 0	no gesture
# 1	quick point
# 2	sustained point
# 3	motion demo
# 4	motion demo + point
# 5	nudge
# 6	guide arm
# 7	do step fully

# task completion:
# 0	not completed
# 1	bad execution
# 2	good execution


def toTime(str):
    #converts str to time in seconds. str in format of mm:ss.mss, e.g. 00:26.803
    assert(len(str) == 9)
    l = str.split(':')
    mm = int(l[0])
    ss = (60. * mm) + float(l[1])
    if ss > 0:
        return ss
    else:
        return -3 #only allows positive time stamps, error code -3 is used otherwise

def convertList(l):
    #converts a list from strings to integers
    newList = list(l)
    for idx, item in enumerate(newList):
#         print ('item', item) #debug
        if item == '\\':
            newList[idx] = -2
        elif item == '':
            newList[idx] = -1
        else:
            if item.isdigit():
                newList[idx] = int(item)
            else:
                time1 = toTime(item)
                assert(time1 > 0)
                newList[idx] = time1
    return newList

def getRowName (R_verbal, R_gesture, P_verbal, P_gesture):
    rowName = ''
    if R_verbal<=0 and R_gesture<=0:
        if (P_verbal==1) and (P_gesture<=0 or P_gesture==1 or P_gesture==2):
            rowName = 'R0Pv1g012'
        elif (P_verbal<=0) and (P_gesture==1 or P_gesture==2):
            rowName = 'R0Pv0g12'
        elif (P_verbal==2) and (P_gesture<=0 or P_gesture==1 or P_gesture==2):
            rowName = 'R0Pv2g012'
        elif (P_verbal<=0 or P_verbal==2) and (P_gesture==3 or P_gesture==4):
            rowName = 'R0Pv02g34'
        elif (P_verbal<=0 or P_verbal==2) and (P_gesture==5 or P_gesture==6):
            rowName = 'R0Pv02g56'
        elif (P_verbal<=0 or P_verbal==2) and (P_gesture==7):
            rowName = 'R0Pv02g7'
    elif R_verbal>0 and R_gesture>0:
        if (P_verbal<=0) and (P_gesture<=0):
            rowName = 'R1Pv0g0'
        elif (P_verbal==1) and (P_gesture<=0 or P_gesture==1 or P_gesture==2):
            rowName = 'R1Pv1g012'
        elif (P_verbal<=0) and (P_gesture==1 or P_gesture==2):
            rowName = 'R1Pv0g12'
        elif (P_verbal==2) and (P_gesture<=0 or P_gesture==1 or P_gesture==2):
            rowName = 'R1Pv2g012'
        elif (P_verbal<=0 or P_verbal==2) and (P_gesture==3 or P_gesture==4):
            rowName = 'R1Pv02g34'
        elif (P_verbal<=0 or P_verbal==2) and (P_gesture==5 or P_gesture==6):
            rowName = 'R1Pv02g56'
        elif (P_verbal<=0 or P_verbal==2) and (P_gesture==7):
            rowName = 'R1Pv02g7'
    if (len(rowName)<=0):
        print('R_verbal', R_verbal)
        print('R_gesture', R_gesture)
        print('P_verbal', P_verbal)
        print('P_gesture', P_gesture)
    assert(len(rowName)>0)
    return rowName

def correctResponse_updateMatrix (step, extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot, number_of_prompts_till_C_stops_correct_step_parent, number_of_prompts_till_C_stops_correct_step_robot, nRinsePrompts, nRinsePromptsComplied):
    numP = max(number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot, 0)
    if step in extendSteps:
        numP = numP + max(number_of_prompts_till_C_stops_correct_step_parent, number_of_prompts_till_C_stops_correct_step_robot, 0)
    if step == 3: #soap step
        if number_of_prompts_till_C_stops_correct_step_parent > 0 or number_of_prompts_till_C_stops_correct_step_robot > 0:
            numP = numP+1 #we say one of them have gotten C to stop soap
            cc2 = colNames.index('wrongStep2wrongStep_Same') #we say the rest of them were ignored by C
            numP2 = max(number_of_prompts_till_C_stops_correct_step_parent-1, number_of_prompts_till_C_stops_correct_step_robot-1, 0)
            responseMatrix[rr, cc2] = responseMatrix[rr, cc2] + numP2
    responseMatrix[rr,cc] = responseMatrix[rr,cc] + numP
    
    if step == 4 or step == 5: #this is rinse / scrub, wanna count rinse prompt compliance (treating scrub same as rinse)
        nRinsePrompts[0] = nRinsePrompts[0] + numP
        nRinsePromptsComplied[0] = nRinsePromptsComplied[0] + numP
    
def incorrectResponse_updateMatrix (step, rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot, nRinsePrompts, nRinsePromptsComplied):
    numP = max(number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot, 0)
    responseMatrix[rr,cc] = responseMatrix[rr,cc] + numP
    
    if step == 4 or step == 5: #this is rinse / scrub, wanna count rinse prompt compliance (treating scrub same as rinse)
        nRinsePrompts[0] = nRinsePrompts[0] + numP

def getSelectedIndx(colNames, selectedColNames):
    indx = []
    for name in selectedColNames:
        indx.append(colNames.index(name))
    return indx

def isSameStep(step1, step2):
    sameRinseSteps = [4, 5, 9] #these steps are considered equivalent: rinse, wet, scrub
    return step1==step2 or (step1 in sameRinseSteps and step2 in sameRinseSteps)

nDistinctSteps = 5. #there can be five distinct steps in each trial: turn on water, --wet-hands-- (not a mandatory step), get soap, [scrub, rinse] (counted as same step), turn off water, dry hands.

def computeMeasures(filename):
    ##reading the table
#     print (filename) #debug
    with open(filename, 'rb') as csvfile:
        content = csv.reader(csvfile, delimiter=',', quotechar='|')
        for row in content:
            #put rows into different variables
#             print ('row[0]', row[0]) #debug
            if row[0] == 'parent involvement':
                assert(row[1].isdigit())
                parent_involvement = int(row[1])
            elif row[0] == 'step':
                step = convertList(row[1:])
            elif row[0] == 'attempted step before prompt':
                attempted_step_before_prompt = convertList(row[1:])
            elif row[0] == 'attempted step successfully executed before prompt':
                attempted_step_successfully_executed_before_prompt = convertList(row[1:])
            elif row[0] == 'P / R / P+R':
                P_R_PandR = convertList(row[1:])
            elif row[0] == 'P verbal':
                P_verbal = convertList(row[1:])
            elif row[0] == 'P gesture':
                P_gesture = convertList(row[1:])
            elif row[0] == 'P reward':
                P_reward = convertList(row[1:])
            elif row[0] == 'R verbal':
                R_verbal = convertList(row[1:])
            elif row[0] == 'R gesture':
                R_gesture = convertList(row[1:])
            elif row[0] == 'R attention grabber':
                R_attention_grabber = convertList(row[1:])
            elif row[0] == 'R reward':
                R_reward = convertList(row[1:])
            elif row[0] == 'C looks at P/R':
                C_looks_at_P_R = convertList(row[1:])
            elif row[0] == 'C smiles':
                C_smiles = convertList(row[1:])
            elif row[0] == 'C murmurs':
                C_murmurs = convertList(row[1:])
            elif row[0] == 'C distracted':
                C_distracted = convertList(row[1:])
            elif row[0] == 'attempted step after prompt':
                attempted_step_after_prompt = convertList(row[1:])
            elif row[0] == 'attempted step successfully executed after prompt':
                attempted_step_successfully_executed_after_prompt = convertList(row[1:])
            elif row[0] == 'attempted step is correct although different from prompt':
                attempted_step_is_correct_although_different_from_prompt = convertList(row[1:])
            elif row[0] == 'time 1st prompt':
                time_1st_prompt = convertList(row[1:])
            elif row[0] == 'time C attempts step':
                time_C_attempts_step = convertList(row[1:])
            elif row[0] == 'time C stops step':
                time_C_stops_step = convertList(row[1:])
            elif row[0] == 'C stops correct step before next prompt':
                C_stops_correct_step_before_next_prompt = convertList(row[1:])
            elif row[0] == 'number of presses till C stops soap':
                number_of_presses_till_C_stops_soap = convertList(row[1:])
            elif row[0] == 'number of prompts till C executes correct step - parent':
                number_of_Prompts_till_C_executes_correct_step_parent = convertList(row[1:])
            elif row[0] == 'number of prompts till C executes correct step - robot':
                number_of_Prompts_till_C_executes_correct_step_robot = convertList(row[1:])
            elif row[0] == 'number of prompts till C stops correct step - parent':
                number_of_prompts_till_C_stops_correct_step_parent = convertList(row[1:])
            elif row[0] == 'number of prompts till C stops correct step - robot':
                number_of_prompts_till_C_stops_correct_step_robot = convertList(row[1:])                
#             print ', '.join(row) #debug
    
    ##initializing
    nPromptSessions = len(step)
    
    sumNumPromptsPhysical_parent = 0
    
    distinct_steps = [[2], [3], [4], [5], [6], [7], [9]]
    distinct_steps_completed_overall = [0, 0, 0, 0, 0, 0, 0]
    distinct_steps_p_phy_int = [0, 0, 0, 0, 0, 0, 0]
    distinct_steps_completed_wo_p_phy_int = [0, 0, 0, 0, 0, 0, 0]
    number_of_distinct_steps_completed_overall = 0
    number_of_distinct_steps_completed_wo_p_phy_int = 0
    
    extendSteps = [4, 5, 7, 9]
    #extendSteps are steps that a prompt a repeated so C does the step for a extended duration of time.  It includes [scrub, rinse, dry, wet].  It's used here so we know which prompts C complied to (in scenarios of right step => right step extend and same vs. right step => right step non-extend or extend and different)
    
    rowNames = ['R0Pv1g012','R0Pv0g12','R0Pv2g012','R0Pv02g34','R0Pv02g56','R0Pv02g7','R1Pv0g0','R1Pv1g012','R1Pv0g12','R1Pv2g012','R1Pv02g34','R1Pv02g56','R1Pv02g7']
    #rows:
#     0   R[0] P verbal[1]gesture[0,1,2][about robot]
#     1   R[0] P verbal[0]gesture[1,2][about object without talking]
#     2   R[0] P verbal[2]gesture[0,1,2][about object with talking]
#     3   R[0] P verbal[0,2]gesture[3,4][about step with salient gestures]
#     4   R[0] P verbal[0,2]gesture[5,6][about compliance with more influence]
#     5   R[0] P verbal[0,2]gesture[7][about compliance with force]
#     6   R[1] P verbal[0]gesture[0] [robot alone]
#     7   R[1] P verbal[1]gesture[0,1,2][about robot with R]
#     8   R[1] P verbal[0]gesture[1,2][about object without talking with R]
#     9   R[1] P verbal[2]gesture[0,1,2][about object with talking with R]
#     10  R[1] P verbal[0,2]gesture[3,4][about step with salient gestures with R]
#     11  R[1] P verbal[0,2]gesture[5,6][about compliance with more influence with R]
#     12  R[1] P verbal[0,2]gesture[7][about compliance with force with R]
    colNames = ['noStep2noStep','noStep2rightStep','noStep2wrongStep','rightStep2rightStep_nonExtend','rightStep2rightStep_extend','rightStep2wrongStep','rightStep2noStep','wrongStep2wrongStep_Diff','wrongStep2wrongStep_Same','wrongStep2rightStep','wrongStep2noStep']
    #columns:
#     0   noStep2noStep 23
#     1   noStep2rightStep 12
#     2   noStep2wrongStep 22
#     3   rightStep2rightStep_nonExtend
#     4   rightStep2rightStep_extend 13
#     5   rightStep2wrongStep 22
#     6   rightStep2noStep 23
#     7   wrongStep2wrongStep_Diff 22
#     8   wrongStep2wrongStep_Same 22
#     9   wrongStep2rightStep 12
#     10  wrongStep2noStep 23
    responseMatrix = np.zeros((len(rowNames), len(colNames)), dtype=int)
    
    nRinsePrompts = [0] #made as a list so can be modified by functions later on when passing in as arg
    nRinsePromptsComplied = [0]
    
    durationRinseLongest = 0
    nPromptSessionsRinse = 0
    
    notWaitingForTurnOnWater = False
    nPromptsBeforeTerminatingSoap = -1
    nPressesTillTerminatingSoap = -1


    ##main loop
    for idx in range(0, nPromptSessions): #looping through prompt sessions
        if step[idx] == 1 or step[idx] == 8: #this is intro or alldone step, skip
            continue
        
        p_phy_int = P_gesture[idx]>= 6 #parent physical intervention flag, 6 is guide arm, 7 is fully do step
        stepCompleted = -1
        if attempted_step_successfully_executed_before_prompt[idx] >= 1:
            stepCompleted = attempted_step_before_prompt[idx]
        elif attempted_step_successfully_executed_after_prompt[idx] >= 1:
            assert(attempted_step_successfully_executed_before_prompt[idx] < 1)
            stepCompleted = attempted_step_after_prompt[idx]
        for stepIdx, distinct_step in enumerate(distinct_steps):
            #distinct steps completed count
            if stepCompleted in distinct_step:
                if p_phy_int:
                    distinct_steps_p_phy_int[stepIdx] = 1
                distinct_steps_completed_overall[stepIdx] = 1
                distinct_steps_completed_wo_p_phy_int[stepIdx] = distinct_steps_p_phy_int[stepIdx] == 0
        
        if P_gesture[idx] >= 6: #P is physically intervening
            sumNumPromptsPhysical_parent = sumNumPromptsPhysical_parent + max(number_of_Prompts_till_C_executes_correct_step_parent[idx], 0) #only count prompts before attempt, since usually physical gestures are not for continuing an extend step, so let's just ignore them
    
        if step[idx] in [4,5] and (attempted_step_before_prompt[idx] in [4,5] or attempted_step_after_prompt[idx] in [4,5]): #if child is executing rinse or scrub step, we wanna measure longest duration before child self terminate
            durationRinseLongest = max(durationRinseLongest, time_C_stops_step[idx] - time_C_attempts_step[idx])
            nPromptSessionsRinse = nPromptSessionsRinse + 1
            # if time_C_attempts_step[idx] < 0:
            #     a = 0
            # elif time_C_stops_step[idx] < 0:
            #     a = 0
            # elif C_stops_correct_step_before_next_prompt[idx] != 1:
            #     a = 0
        
        #notWaitingForTurnOnWater
        if attempted_step_before_prompt[idx] == 2 or (step[idx] != 2 and  attempted_step_after_prompt[idx] == 2 and attempted_step_is_correct_although_different_from_prompt[idx] < 1):
            notWaitingForTurnOnWater = True
        
        #nPromptsBeforeTerminatingSoap
        if step[idx] == 3:
            if C_stops_correct_step_before_next_prompt[idx] == 1:
                nPromptsBeforeTerminatingSoap = 0
            else:
                nPromptsBeforeTerminatingSoap = max(nPromptsBeforeTerminatingSoap, number_of_prompts_till_C_stops_correct_step_parent[idx], number_of_prompts_till_C_stops_correct_step_robot[idx])
                
        nPressesTillTerminatingSoap = max(nPressesTillTerminatingSoap, number_of_presses_till_C_stops_soap[idx])


        #C response to prompts
        if P_R_PandR[idx] > 0: #if prompted
            if P_verbal[idx]>0 or P_gesture[idx]>0 or R_verbal[idx]>0 or R_gesture[idx]>0: #Make sure P / R is giving a prompt, not just an AG or reward, i.e. either P or R should have it's verbal and/or gesture prompt marked as some positive value
            
                stepKind_before_prompt = -2 #stepKind = -1 if noStep, 0 if wrongStep, 1 if rightStep
                if attempted_step_before_prompt[idx] == 0: #noStep
                    stepKind_before_prompt = -1
                else:
                    atmpStep = -1
                    if attempted_step_before_prompt[idx] == -1: #not marked, use previous section's attempted step after prompt
                        if attempted_step_after_prompt[idx-1] == -1: #not marked, use previous section's attempted step before prompt
                            atmpStep = attempted_step_before_prompt[idx-1]
                        else:
                            atmpStep = attempted_step_after_prompt[idx-1]
                    else:
                        atmpStep = attempted_step_before_prompt[idx]
                    assert(atmpStep!=-1)
                    if isSameStep(atmpStep, step[idx]): #rightStep
                        stepKind_before_prompt = 1
                    else: #wrongStep
                        stepKind_before_prompt = 0
                assert(stepKind_before_prompt != -2)
                
                assert(attempted_step_after_prompt[idx]>=0) #we shouldn't have a '\' or '' (i.e. not marked)
                
                stepKind_after_prompt = -2
                if attempted_step_after_prompt[idx] == 0: #noStep
                    stepKind_after_prompt = -1
                elif isSameStep(attempted_step_after_prompt[idx], step[idx]) or attempted_step_is_correct_although_different_from_prompt[idx] == 1: #rightStep
                    stepKind_after_prompt = 1
                else: #wrongStep
                    stepKind_after_prompt = 0
                assert(stepKind_after_prompt!=-2)
                
                if stepKind_before_prompt == -1 and stepKind_after_prompt == -1:					
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('noStep2noStep')
                    incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                elif stepKind_before_prompt == -1 and stepKind_after_prompt == 1:					
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('noStep2rightStep')
                    correctResponse_updateMatrix(step[idx], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], number_of_prompts_till_C_stops_correct_step_parent[idx], number_of_prompts_till_C_stops_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                elif stepKind_before_prompt == -1 and stepKind_after_prompt == 0:
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('noStep2wrongStep')
                    incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                elif stepKind_before_prompt == 0 and stepKind_after_prompt == -1:
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('wrongStep2noStep')
                    incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                elif stepKind_before_prompt == 0 and stepKind_after_prompt == 1:
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('wrongStep2rightStep')
                    correctResponse_updateMatrix(step[idx], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], number_of_prompts_till_C_stops_correct_step_parent[idx], number_of_prompts_till_C_stops_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                elif stepKind_before_prompt == 0 and stepKind_after_prompt == 0:
                    if attempted_step_before_prompt[idx] != attempted_step_after_prompt[idx]:
                        rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                        cc = colNames.index('wrongStep2wrongStep_Diff')
                        incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                        
                    elif attempted_step_before_prompt[idx] == attempted_step_after_prompt[idx]:
                        rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                        cc = colNames.index('wrongStep2wrongStep_Same')
                        incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                
                elif stepKind_before_prompt == 1 and stepKind_after_prompt == -1:
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('rightStep2noStep')
                    incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                elif stepKind_before_prompt == 1 and stepKind_after_prompt == 1:
                    if attempted_step_after_prompt[idx] in extendSteps and attempted_step_before_prompt[idx] == attempted_step_after_prompt[idx]:
                        rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                        cc = colNames.index('rightStep2rightStep_extend')
                        correctResponse_updateMatrix(step[idx], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], number_of_prompts_till_C_stops_correct_step_parent[idx], number_of_prompts_till_C_stops_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                        
                    elif attempted_step_after_prompt[idx] not in extendSteps:
                        rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                        cc = colNames.index('rightStep2rightStep_nonExtend')
                        correctResponse_updateMatrix(step[idx], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], number_of_prompts_till_C_stops_correct_step_parent[idx], number_of_prompts_till_C_stops_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                        
                elif stepKind_before_prompt == 1 and stepKind_after_prompt == 0:
                    rr = rowNames.index(getRowName(R_verbal[idx], R_gesture[idx], P_verbal[idx], P_gesture[idx]))
                    cc = colNames.index('rightStep2wrongStep')
                    incorrectResponse_updateMatrix(step[idx], rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[idx], number_of_Prompts_till_C_executes_correct_step_robot[idx], nRinsePrompts, nRinsePromptsComplied)
                    
                else:
                    assert(False)
    
    
    ##outputing
    retDict = collections.OrderedDict()	
    smplSzDict = collections.OrderedDict()
    
    retDict['Number of Steps Completed - Overall'] = sum(distinct_steps_completed_overall)
    smplSzDict['Number of Steps Completed - Overall'] = nPromptSessions
    # retDict['Number of Steps Completed - Without Parent Phys. Intervene'] = sum(distinct_steps_completed_wo_p_phy_int)
    # smplSzDict['Number of Steps Completed - Without Parent Phys. Intervene'] = nPromptSessions
    
    no_effects_from_prompt_indx = getSelectedIndx(colNames, ['rightStep2rightStep_nonExtend', 'wrongStep2wrongStep_Same', 'noStep2noStep'])
    stopped_for_no_reason_indx = getSelectedIndx(colNames, ['rightStep2noStep', 'wrongStep2noStep'])
    special_case_indx = getSelectedIndx(colNames, ['rightStep2wrongStep', 'wrongStep2wrongStep_Diff', 'noStep2wrongStep'])
    compliance_indx = getSelectedIndx(colNames, ['rightStep2rightStep_extend', 'wrongStep2rightStep', 'noStep2rightStep'])
    compliance_hard_indx = getSelectedIndx(colNames, ['wrongStep2rightStep'])
    
    no_effects_from_prompt_SmplSz_indx = getSelectedIndx(colNames, colNames)
    stopped_for_no_reason_SmplSz_indx = getSelectedIndx(colNames, colNames)
    special_case_SmplSz_indx = getSelectedIndx(colNames, colNames)
    compliance_SmplSz_indx = getSelectedIndx(colNames, colNames)
    compliance_hard_SmplSz_indx = getSelectedIndx(colNames, ['wrongStep2rightStep', 'wrongStep2wrongStep_Diff', 'wrongStep2wrongStep_Same', 'wrongStep2noStep'])
    
    no_effects_from_prompt_rate_v_SmplSz = np.sum(responseMatrix[:, no_effects_from_prompt_SmplSz_indx], 1)
    no_effects_from_prompt_rate_v = 1. * np.sum(responseMatrix[:, no_effects_from_prompt_indx], 1) / no_effects_from_prompt_rate_v_SmplSz
    
    stopped_for_no_reason_rate_v_SmplSz = np.sum(responseMatrix[:, stopped_for_no_reason_SmplSz_indx], 1)
    stopped_for_no_reason_rate_v = 1. * np.sum(responseMatrix[:, stopped_for_no_reason_indx], 1) / stopped_for_no_reason_rate_v_SmplSz
    
    special_case_rate_v_SmplSz = np.sum(responseMatrix[:, special_case_SmplSz_indx], 1)
    special_case_rate_v = 1. * np.sum(responseMatrix[:, special_case_indx], 1) / special_case_rate_v_SmplSz
    
    compliance_rate_v_SmplSz = np.sum(responseMatrix[:, compliance_SmplSz_indx], 1)
    compliance_rate_v = 1. * np.sum(responseMatrix[:, compliance_indx], 1) / compliance_rate_v_SmplSz
    
    compliance_hard_rate_v_SmplSz = np.sum(responseMatrix[:, compliance_hard_SmplSz_indx], 1)
    compliance_hard_rate_v = 1. * np.sum(responseMatrix[:, compliance_hard_indx], 1) / compliance_hard_rate_v_SmplSz
        
    # print('no_effects_from_prompt_rate_v', no_effects_from_prompt_rate_v)
    # print('no_effects_from_prompt_rate_v_SmplSz', no_effects_from_prompt_rate_v_SmplSz)	
    # print('stopped_for_no_reason_rate_v', stopped_for_no_reason_rate_v)
    # print('stopped_for_no_reason_rate_v_SmplSz', stopped_for_no_reason_rate_v_SmplSz)
    # print('special_case_rate_v', special_case_rate_v)
    # print('special_case_rate_v_SmplSz', special_case_rate_v_SmplSz)
    # print('compliance_rate_v', compliance_rate_v)
    # print('compliance_rate_v_SmplSz', compliance_rate_v_SmplSz)
    # print('compliance_hard_rate_v', compliance_hard_rate_v)
    # print('compliance_hard_rate_v_SmplSz', compliance_hard_rate_v_SmplSz)
    
        
    # for ii, value in enumerate(no_effects_from_prompt_rate_v):
    #     retDict['Ignored Prompt Rate - ' + rowNames[ii]] = value
    # for ii, value in enumerate(no_effects_from_prompt_rate_v_SmplSz):
    #     smplSzDict['Ignored Prompt Rate - ' + rowNames[ii]] = value
    
    # for ii, value in enumerate(stopped_for_no_reason_rate_v):
    #     retDict['Stopped For No Reason Rate - ' + rowNames[ii]] = value
    # for ii, value in enumerate(stopped_for_no_reason_rate_v_SmplSz):
    #     smplSzDict['Stopped For No Reason Rate - ' + rowNames[ii]] = value
    
    # for ii, value in enumerate(special_case_rate_v):
    #     retDict['Confused By Prompt Rate - ' + rowNames[ii]] = value
    # for ii, value in enumerate(special_case_rate_v_SmplSz):
    #     smplSzDict['Confused By Prompt Rate - ' + rowNames[ii]] = value
    
    # for ii, value in enumerate(compliance_rate_v):
    #     retDict['Complied Prompt Rate - ' + rowNames[ii]] = value
    # for ii, value in enumerate(compliance_rate_v_SmplSz):
    #     smplSzDict['Complied Prompt Rate - ' + rowNames[ii]] = value
    
    # for ii, value in enumerate(compliance_hard_rate_v):
    #     retDict['Hard Complied Prompt Rate - ' + rowNames[ii]] = value
    # for ii, value in enumerate(compliance_hard_rate_v_SmplSz):
    #     smplSzDict['Hard Complied Prompt Rate - ' + rowNames[ii]] = value
    
    responseMatrix_overall = np.sum(responseMatrix, 0)
    
    # rightStep2rightStep_nonExtend_dbg = responseMatrix_overall[getSelectedIndx(colNames, ['rightStep2rightStep_nonExtend'])]
    
    no_effects_from_prompt_rate_overall_SmplSz = np.sum(responseMatrix_overall[no_effects_from_prompt_SmplSz_indx])
    no_effects_from_prompt_rate_overall = 1. * np.sum(responseMatrix_overall[no_effects_from_prompt_indx]) / np.array(no_effects_from_prompt_rate_overall_SmplSz) # / np.array(*) gets rid of division by zero error (automatically places a NaN)
    
    stopped_for_no_reason_rate_overall_SmplSz = np.sum(responseMatrix_overall[stopped_for_no_reason_SmplSz_indx])
    stopped_for_no_reason_rate_overall = 1. * np.sum(responseMatrix_overall[stopped_for_no_reason_indx]) / np.array(stopped_for_no_reason_rate_overall_SmplSz)
    
    special_case_rate_overall_SmplSz = np.sum(responseMatrix_overall[special_case_SmplSz_indx])
    special_case_rate_overall = 1. * np.sum(responseMatrix_overall[special_case_indx]) / np.array(special_case_rate_overall_SmplSz)
    
    compliance_rate_overall_SmplSz = np.sum(responseMatrix_overall[compliance_SmplSz_indx])
    compliance_rate_overall = 1. * np.sum(responseMatrix_overall[compliance_indx]) / np.array(compliance_rate_overall_SmplSz)
    
    compliance_hard_rate_overall_SmplSz = np.sum(responseMatrix_overall[compliance_hard_SmplSz_indx])
    compliance_hard_rate_overall = 1. * np.sum(responseMatrix_overall[compliance_hard_indx]) / np.array(compliance_hard_rate_overall_SmplSz)
    
    # print('responseMatrix', responseMatrix)
    # print('responseMatrix_overall', responseMatrix_overall)
    
    # print('no_effects_from_prompt_rate_overall', no_effects_from_prompt_rate_overall)
    # print('no_effects_from_prompt_rate_overall_SmplSz', no_effects_from_prompt_rate_overall_SmplSz)
    # print('stopped_for_no_reason_rate_overall', stopped_for_no_reason_rate_overall)
    # print('stopped_for_no_reason_rate_overall_SmplSz', stopped_for_no_reason_rate_overall_SmplSz)
    # print('special_case_rate_overall', special_case_rate_overall)
    # print('special_case_rate_overall_SmplSz', special_case_rate_overall_SmplSz)
    # print('compliance_rate_overall', compliance_rate_overall)
    # print('compliance_rate_overall_SmplSz', compliance_rate_overall_SmplSz)
    # print('compliance_hard_rate_overall', compliance_hard_rate_overall)
    # print('compliance_hard_rate_overall_SmplSz', compliance_hard_rate_overall_SmplSz)

    # print('\n')

    retDict['Ignored Prompt Rate - Overall'] = no_effects_from_prompt_rate_overall
    smplSzDict['Ignored Prompt Rate - Overall'] = no_effects_from_prompt_rate_overall_SmplSz
    retDict['Stopped For No Reason Rate - Overall'] = stopped_for_no_reason_rate_overall
    smplSzDict['Stopped For No Reason Rate - Overall'] = stopped_for_no_reason_rate_overall_SmplSz
    retDict['Confused By Prompt Rate - Overall'] = special_case_rate_overall
    smplSzDict['Confused By Prompt Rate - Overall'] = special_case_rate_overall_SmplSz
    retDict['Complied Prompt Rate - Overall'] = compliance_rate_overall
    smplSzDict['Complied Prompt Rate - Overall'] = compliance_rate_overall_SmplSz
    # retDict['Hard Complied Prompt Rate - Overall'] = compliance_hard_rate_overall    
    # smplSzDict['Hard Complied Prompt Rate - Overall'] = compliance_hard_rate_overall_SmplSz
    
    retDict['Complied Prompt Rate - Rinse Step'] = 1. * np.array(nRinsePromptsComplied) / np.array(nRinsePrompts)
    smplSzDict['Complied Prompt Rate - Rinse Step'] = nRinsePrompts[0]
    
    retDict['Parent Involvement'] = parent_involvement
    smplSzDict['Parent Involvement'] = 1
    
    retDict['Number of Parent Prompts - Phys Interv'] = sumNumPromptsPhysical_parent
    smplSzDict['Number of Parent Prompts - Phys Interv'] = nPromptSessions
    
    retDict['Longest Duration of Rinse (sec)'] = durationRinseLongest
    smplSzDict['Longest Duration of Rinse (sec)'] = nPromptSessionsRinse
    
    retDict['Not Waiting For Turn On Water'] = notWaitingForTurnOnWater
    smplSzDict['Not Waiting For Turn On Water'] = 1
    
    # if nPromptsBeforeTerminatingSoap == -1:
    #     nPromptsBeforeTerminatingSoap = np.NaN
    #     a = 1
    # retDict['Number of Prompts Before Terminating Soap'] = nPromptsBeforeTerminatingSoap
    # smplSzDict['Number of Prompts Before Terminating Soap'] = 1
    # 
    # if nPressesTillTerminatingSoap < 0:
    #     nPressesTillTerminatingSoap = np.NaN
    # retDict['Number of Presses Before Terminating Soap'] = nPressesTillTerminatingSoap
    # smplSzDict['Number of Presses Before Terminating Soap'] = 1
    
    return [retDict, smplSzDict]
    


### script start

##load the files
directory = 'C:\\Users\\David\\Documents\\GitHub\\MASc\\data analysis\\analysis after annotation\\'
names_t1 = ['t1-s1.csv', 't1-s2.csv', 't1-s3.csv', 't1-s4.csv', 't1-s5.csv', 't1-s6.csv', 't1-s7.csv', 't1-s8.csv']
names_t2 = ['t2-s1.csv', 't2-s2.csv', 't2-s3.csv', 't2-s4.csv', 't2-s5.csv', 't2-s6.csv', 't2-s7.csv', 't2-s8.csv']
names_t3 = ['t3-s1.csv', 't3-s2.csv', 't3-s3.csv', 't3-s4.csv', 't3-s5.csv', 't3-s6.csv']
names_t4 = ['t4-s1.csv', 't4-s2.csv', 't4-s3.csv', 't4-s4.csv', 't4-s5.csv', 't4-s6.csv', 't4-s7.csv', 't4-s8.csv', 't4-s9.csv']
names_t5 = ['t5-s1.csv', 't5-s2.csv', 't5-s3.csv', 't5-s4.csv', 't5-s5.csv', 't5-s6.csv', 't5-s7.csv', 't5-s8.csv', 't5-s9.csv']
names_t6 = ['t6-s1.csv', 't6-s2.csv', 't6-s3.csv', 't6-s4.csv', 't6-s5.csv', 't6-s6.csv', 't6-s7.csv', 't6-s8.csv', 't6-s9.csv', 't6-s10.csv']
names_all = [names_t1, names_t2, names_t3, names_t4, names_t5, names_t6]
# names_all = [['t6-s1.csv']] #debug

##getting the results
resultsDict = collections.OrderedDict()
resultsSmplSzDict = collections.OrderedDict()
currRowFirstIdx = 0 #we are aggregating all the trials from different "rows" (visits) together, so need a IdxPtr to keep track of the beginning of each row
for i, names in enumerate(names_all):
    for j, filename in enumerate(names):
        print ('filename', filename)
        result, resultSmplSz = computeMeasures(directory + filename)
        
        if len(resultsDict.keys()) == 0: #need to initialize the dict and create lists        
            resultsDict = collections.OrderedDict.fromkeys(result.keys())
            assert(cmp(result.keys(), resultSmplSz.keys() == 0))
            resultsSmplSzDict = collections.OrderedDict.fromkeys(result.keys())            
            #calculate total number of points (sessions) and set resultsDict as a dict arrays of that size
            nTrials = 0
            for names1 in names_all:
                nTrials = nTrials + len(names1)
            for key1 in resultsDict:
                resultsDict[key1] = np.zeros(nTrials)
                resultsSmplSzDict[key1] = np.zeros(nTrials, dtype=int)

        for key2 in result:
            if key2 == 'Not Waiting For Turn On Water':
                a = 1
            elif key2 == 'Number of Prompts Before Terminating Soap':
                a = 1
            resultsDict[key2][currRowFirstIdx + j] = result[key2]
            resultsSmplSzDict[key2][currRowFirstIdx + j] = resultSmplSz[key2]
    currRowFirstIdx = currRowFirstIdx + len(names)


##plotting

#phase segments
phaseSegs = [16, 8, 21, 5]
phaseLabel = ['A', 'B1', 'C', 'B2']

plt.figure(1)
timeAxis = np.arange(1, len(resultsDict[resultsDict.keys()[0]])+1)

for i, key3 in enumerate(resultsDict):

    if 'Not Waiting For Turn On Water' in key3:
        phaseEndIndx = 0
        for phaseIdx, phaseLen in enumerate(phaseSegs):
            newPhaseEndIndx = phaseEndIndx + phaseLen
            indices = range(phaseEndIndx, newPhaseEndIndx)
            rez = resultsDict['Not Waiting For Turn On Water'][indices]
            print('Not Waiting For Turn On Water - Phase ' + phaseLabel[phaseIdx] + ': ' + str(sum(rez)) + '/' + str(len(rez)) + ' = ' + str(np.average(rez)))
            phaseEndIndx = newPhaseEndIndx
        continue

    plt.clf()
    plt.rc("font", size=18)
    
    phaseEndIndx = 0
    for phaseIdx, phaseLen in enumerate(phaseSegs):
        #form the indices of items of the phase
        newPhaseEndIndx = phaseEndIndx + phaseLen
        indices = range(phaseEndIndx, newPhaseEndIndx)
        plt.plot(timeAxis[indices], resultsDict[key3][indices], color='k', marker='.', linestyle='-', linewidth=0.8, markersize=10)
        #draw vertical phase lines
        if phaseIdx != len(phaseSegs)-1: #don't draw line for the last seg
            plt.axvline(newPhaseEndIndx + 0.5, color='k')
        #draw median lines
        if 'Parent Involvement' not in key3 and 'Number of Parent Prompts' not in key3:
            x = resultsDict[key3][indices]
            x = x[~np.isnan(x)] #get rid of the NaNs when calculating the median
            med = np.median(x)
            plt.plot([timeAxis[indices[0]], timeAxis[indices[-1]]], [med, med], color='k', linestyle='-', linewidth=2.5)
            
            #########################
            ##median values labeled##
            #########################
            # plt.text((timeAxis[indices[0]] + timeAxis[indices[-1]])*0.5, med, str(round(med, 2)), fontsize=15) 
            
        phaseEndIndx = newPhaseEndIndx


    #title, axes names, and legends
#     plt.title(key3)
    plt.xlabel('Trial Number')
    plt.ylabel(key3)
    #x,y limits, make sure points don't touch boundary
    plt.xlim(0.5, len(timeAxis)+1)
    plt.ylim(ymin=-0.1)
    if 'Rate' in key3 in key3:
        plt.ylim(ymax=1.1)
#         plt.ylim(-1, 1) #todo: debug
    if 'Number of Steps Completed' in key3:
        plt.ylim(ymax=6.5)
    if 'Parent Involvement' in key3:
        plt.ylim(ymax=4.5)
    # plt.ylim(ymax=plt.ylim()[1]+0.1
    # annotate phases
    phaseEndIndx = 0
    for phaseIdx, phaseLen in enumerate(phaseSegs):
        newPhaseEndIndx = phaseEndIndx + phaseLen
        plt.annotate(phaseLabel[phaseIdx], xy=(0.5*(phaseEndIndx+newPhaseEndIndx), 0.95*plt.ylim()[1]+0.05*plt.ylim()[0]), color='k', size=20)
        phaseEndIndx = newPhaseEndIndx
    # plt.show()
    plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ()') + '.eps') #no spaces in names for latex use
    plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ()') + '.png') #no spaces in names for latex use
    



##Step Completion vs. Compliance
# plt.clf()
# plt.rc("font", size=18)
# colors = ['r', 'k', 'g', 'b']
# markers = ['^', 's', 'o', 'h']
# # xconditions = ['Complied Prompt Rate - Overall', 'Complied Prompt Rate - R1Pv0g0']
# # yconditions = ['Number of Steps Completed - Overall', 'Number of Steps Completed - Without Parent Phys. Intervene']
# xconditions = ['Complied Prompt Rate - Overall']
# yconditions = ['Number of Steps Completed - Overall']
# legendHandels = []
# for xcond in xconditions:
#     for ycond in yconditions:
#         phaseEndIndx = 0
#         for phaseIdx, phaseLen in enumerate(phaseSegs):
#             #form the indices of items of the phase
#             newPhaseEndIndx = phaseEndIndx + phaseLen
#             indices = range(phaseEndIndx, newPhaseEndIndx)
#             # print (indices) #debug
#             h, = plt.plot(resultsDict[xcond][indices], resultsDict[ycond][indices], color=colors[phaseIdx], marker=markers[phaseIdx], linestyle='', linewidth=0.8, markersize=10)
#             legendHandels.append(h)
#             phaseEndIndx = newPhaseEndIndx
#             
#             #label axes
#             plt.ylim(ymax=6, ymin=-1)
#             plt.xlim(xmax=1.1, xmin=-0.1)
#             plt.xlabel('Complied Prompt Rate - Overall')
#             plt.ylabel('Number of Steps Completed - Overall')
#             
#             #legends
#             plt.legend(legendHandels, phaseLabel, loc='lower right', fontsize=10, shadow=True, markerscale=0.75, numpoints=1)
#             
#             #save figures
#             plt.savefig(directory + 'figures\\' + ycond.translate(None, ' ') + '_vs_' + xcond.translate(None, ' ') + str(phaseIdx) + '.eps') #no spaces in names for latex use
#             plt.savefig(directory + 'figures\\' + ycond.translate(None, ' ') + '_vs_' + xcond.translate(None, ' ') + str(phaseIdx) + '.png') #no spaces in names for latex use
#         # plt.show() #debug
#             
