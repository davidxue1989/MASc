import csv
import copy
import collections #for ordereddict
import matplotlib.pyplot as plt
import numpy as np

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
#         print ('item', item) #todo: debug
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

## Analysis Revision 2.0 - Compliance Rates
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

def correctResponse_updateMatrix (step, extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot, number_of_prompts_till_C_stops_correct_step_parent, number_of_prompts_till_C_stops_correct_step_robot):
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
    
def incorrectResponse_updateMatrix (rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot):
    numP = max(number_of_Prompts_till_C_executes_correct_step_parent, number_of_Prompts_till_C_executes_correct_step_robot, 0)
    responseMatrix[rr,cc] = responseMatrix[rr,cc] + numP
    
def getSelectedIndx(colNames, selectedColNames):
    indx = []
    for name in selectedColNames:
        indx.append(colNames.index(name))
    return indx
    
def isSameStep(step1, step2):
    sameRinseSteps = [4, 5, 9] #these steps are considered equivalent: rinse, wet, scrub
    return step1==step2 or (step1 in sameRinseSteps and step2 in sameRinseSteps)
    
    
##calculating step completion rate
def calcStepCompletionRate(filename):
    #reading the table
#     print (filename)
    with open(filename, 'rb') as csvfile:
        content = csv.reader(csvfile, delimiter=',', quotechar='|')
        for row in content:
            #put rows into different variables
#             print ('row[0]', row[0]) #todo: debug
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
                
#             elif row[0] == 'number of prompts till C executes correct step':
#                 number_of_Prompts_till_C_executes_correct_step = convertList(row[1:])
#             elif row[0] == 'number of prompts till C stops step':
#                 number_of_prompts_till_C_stops_step = convertList(row[1:])
            elif row[0] == 'number of prompts till C executes correct step - parent':
                number_of_Prompts_till_C_executes_correct_step_parent = convertList(row[1:])
            elif row[0] == 'number of prompts till C executes correct step - robot':
                number_of_Prompts_till_C_executes_correct_step_robot = convertList(row[1:])
            elif row[0] == 'number of prompts till C stops correct step - parent':
                number_of_prompts_till_C_stops_correct_step_parent = convertList(row[1:])
            elif row[0] == 'number of prompts till C stops correct step - robot':
                number_of_prompts_till_C_stops_correct_step_robot = convertList(row[1:])
                
#             print ', '.join(row)
        
    nSteps = 0 #number of steps (a.k.a. sectors)
    
    extendSteps = [4, 5, 7, 9]
    #extendSteps are steps that a prompt a repeated so C does the step for a extended duration of time.  It includes [scrub, rinse, dry, wet].  It's used here so we know which prompts C complied to (in scenarios of right step => right step extend and same vs. right step => right step non-extend or extend and different)
    
    
    stepNotAttempted_beforePrompt = 0
    stepAttemptedCount_beforePrompt = 0
    stepCompletedCount_beforePrompt = 0
    
    noStep2noStep = 0
    noStep2rightStep = 0
    noStep2wrongStep = 0
    rightStep2rightStep_nonExtend = 0
    rightStep2rightStep_extend = 0
    rightStep2wrongStep = 0
    rightStep2noStep = 0
    wrongStep2wrongStep_Diff = 0
    wrongStep2wrongStep_Same = 0
    wrongStep2rightStep = 0
    wrongStep2noStep = 0
    
    ## Analysis Revision 2.0 - Compliance Rates
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
    
    ## Analysis Revision 2.0 - Engagement Level
    numCSmiles = 0
    numCDistracted = 0
    numCMurmurs = 0
    
    
    totalNPrompts = 0
    C_looks_P_given_P_prompt = 0
    C_looks_R_given_R_prompt = 0
    C_looks_P_given_PnR_prompt = 0
    C_looks_R_given_PnR_prompt = 0
    nPrompts_P = 0
    nPrompts_R = 0
    nPrompts_PnR = 0
    
    nPVerbals_prompts_step = 0
    nPGestures_is_physical = 0
    
    stepAttemptedCount_afterPrompt = 0
    stepCompletedCount_afterPrompt = 0
    
    stepCompletedCount_overall = 0 #counting all steps completed
    
    
    ## Analysis Revision 2.0 - overall step completion count
    correctStepCompletedCount_total = 0
    correctStepCompletedCount_withoutPhysicalPrompt_total = 0
    correctStepCompletedCount_withoutPPrompts_total = 0
    correctStepCompletedCount_withoutPnRPrompts_total = 0
    
    oneStep = []
    currentStep = 0
    
    
    ## Duration of C's responses
    sumDurationTillCAttemptsCorrectStep = 0
    sumNumTimeCAttemptsCorrectStepAfterPrompted = 0
    sumNumTimeCAttemptsCorrectStepOverall = 0
    
    sumPromptsTillCAttemptsCorrectStep = 0
    
    sumDurationTillCStopsSoap = 0
    sumDurationTillCStopsRinse = 0
    sumDurationTIllCStopsDry = 0
    sumNumTimeCStopsSoap = 0
    sumNumTimeCStopsRinse = 0
    sumNumTimeCStopsDry = 0
    
    sumNumTimeCStopsBeforePrompt = 0
    sumPromptsTillCStopsRinse = 0
    sumPromptsTillCStopsDry = 0
    sumPromptsTillCStopsSoap = 0
    sumNumPressesTillCStopsSoap = 0
    
    ## Analysis Revision 2.0 - number of prompts issued
    sumNumPromptsTotal_parent = 0
    sumNumPromptsPhysical_parent = 0
    sumNumPromptsTotal_robot = 0
    
    
    #main loop
    for idx in range(0, len(step)+1):
        if (idx == len(step) or not isSameStep(step[idx], currentStep)) and len(oneStep) != 0:
        #this is to loop through the steps sector by sector (a sector contains prompt sections for the same step) (i.e. oneStep)
        #process the step and calculate the measures:
            #attempt rate before prompt: count every attempt within a step / per step if not attempted for whole step before prompt or per attempt if attempted more than once in a step
            #attempt rate after prompt: count every attempt within a step / per every prompt entry
            #completion rate before prompt: only for every attempt / per attempt
            #completion rate after prompt: only for every attempt / per attempt
            #completion rate overall: only for every step / per step
                    
#             print (oneStep)
            
            nSteps = nSteps+1
            
            ## C attempts step before prompt
            anyAttemptsThisStep_beforePrompt = False
            ## Step Completion After Prompt
            previousAttemptingStep_afterPrompt = -3 #just some value previousAttemptingStep_afterPrompt will never have
            nContinuousAttemptedSteps_afterPrompt = 0
            nContinuousCompletedSteps_afterPrompt = 0
            
            stepCompletedThisStep_overall = False
            
            ## Analysis Revision 2.0 - overall step completion count
            correctStepCompleted_oneStep = False
            correctStepCompleted_withoutPhysicalPrompts_oneStep = False
            correctStepCompleted_withoutPPrompts_oneStep = False
            correctStepCompleted_withoutPnRPrompts_oneStep = False
            
            ## Duration of C's responses
            promptStartTime = -1
            nPromptsTillCAttemptsCorrectStep = 0
            #assuming soap, rinse, dry all stop within one prompt section, so no need to count their durations across prompt sections
            
            for i in oneStep:
                
                ## overall step completion count            
                if isSameStep(attempted_step_before_prompt[i], step[i]) and attempted_step_successfully_executed_before_prompt[i] >= 1:
                    correctStepCompleted_oneStep = True
                    correctStepCompleted_withoutPhysicalPrompts_oneStep = True
                    correctStepCompleted_withoutPPrompts_oneStep = True
                    correctStepCompleted_withoutPnRPrompts_oneStep = True
                if (isSameStep(attempted_step_after_prompt[i], step[i]) and attempted_step_successfully_executed_after_prompt[i] >= 1) or attempted_step_is_correct_although_different_from_prompt[i] == 1:
                    correctStepCompleted_oneStep = True
                    if P_gesture[i] < 5: #not because of P physical prompts
                        correctStepCompleted_withoutPhysicalPrompts_oneStep = True
                        if P_gesture[i] <= 0 and P_verbal[i] <= 0:
                            correctStepCompleted_withoutPPrompts_oneStep = True
                            if R_gesture[i] <= 0 and R_verbal[i] <= 0:
                                correctStepCompleted_withoutPnRPrompts_oneStep = True
#                                 print('step', step[i])
#                                 print('index', i)
#                                 assert(False) #dxnote: I don't think we'll ever come here, since if no one prompted, attempted_step_after_prompt[i] should be blank ==> not true!  Robot reward still is considered a prompt to stop step
                
                ## C attempts step before prompt
                #find an attempt before prompt
                if attempted_step_before_prompt[i] > 0:
                    anyAttemptsThisStep_beforePrompt = True
                    if attempted_step_successfully_executed_before_prompt[i] >= 0: #don't count the case where attempted_step_successfully_executed_before_prompt[i] == -2 ('\'), i.e. step wasn't complete but wasn't because C fails in executing, but may be because C is waiting for P's approval or C's action is cut off by prompts
                        stepAttemptedCount_beforePrompt = stepAttemptedCount_beforePrompt+1
                        if attempted_step_successfully_executed_before_prompt[i] >= 1:
                            stepCompletedCount_beforePrompt = stepCompletedCount_beforePrompt+1
                            stepCompletedThisStep_overall = True
                
                #if P_R_PandR > 0, i.e. prompted:
                #calculate the following:
                #noStep2noStep
                #noStep2rightStep
                #noStep2wrongStep
                #wrongStep2noStep
                #wrongStep2rightStep
                #wrongStep2wrongStep_Diff
                #wrongStep2wrongStep_Same
                #rightStep2noStep
                #rightStep2rightStep_nonExtend
                #rightStep2rightStep_extend
                #rightStep2wrongStep
                #totalNPrompts
                #assert(totalNPrompts == sum of above variables)
                
                if P_R_PandR[i] > 0:
                    
                    ## C response types
                    
                    #Make sure P / R is giving a prompt, not just an AG or reward, i.e. either P or R should have it's verbal and/or gesture prompt marked as some positive value
                    if P_verbal[i]>0 or P_gesture[i]>0 or R_verbal[i]>0 or R_gesture[i]>0:
                        totalNPrompts = totalNPrompts + 1
                    
                        #stepKind = -1 if noStep, 0 if wrongStep, 1 if rightStep
                        
                        stepKind_before_prompt = -2
                        if attempted_step_before_prompt[i] == 0: #noStep
                            stepKind_before_prompt = -1
                        else:
                            tmpStep = -1
                            if attempted_step_before_prompt[i] == -1: #not marked, use previous section's attempted step after prompt
                                if attempted_step_after_prompt[i-1] == -1: #not marked, use attempted step before prompt
                                    tmpStep = attempted_step_before_prompt[i-1]
                                else:
                                    tmpStep = attempted_step_after_prompt[i-1]
                            else:
                                tmpStep = attempted_step_before_prompt[i]
                            assert(tmpStep!=-1)
                            if isSameStep(tmpStep, step[i]): #rightStep
                                stepKind_before_prompt = 1
                            else: #wrongStep
                                stepKind_before_prompt = 0
                        assert(stepKind_before_prompt != -2)
                        
                        assert(attempted_step_after_prompt[i]>=0) #we shouldn't have a '\' or ''
                        
                        stepKind_after_prompt = -2
                        if attempted_step_after_prompt[i] == 0: #noStep
                            stepKind_after_prompt = -1
                        elif isSameStep(attempted_step_after_prompt[i], step[i]) or attempted_step_is_correct_although_different_from_prompt[i] == 1: #rightStep
                            stepKind_after_prompt = 1
                        else: #wrongStep
                            stepKind_after_prompt = 0
                        assert(stepKind_after_prompt!=-2)
                        
                        
                        if stepKind_before_prompt == -1 and stepKind_after_prompt == -1:
                            noStep2noStep = noStep2noStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('noStep2noStep')
                            incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                            
                        elif stepKind_before_prompt == -1 and stepKind_after_prompt == 1:
                            noStep2rightStep = noStep2rightStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('noStep2rightStep')
                            correctResponse_updateMatrix(step[i], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i], number_of_prompts_till_C_stops_correct_step_parent[i], number_of_prompts_till_C_stops_correct_step_robot[i])
                            
                        elif stepKind_before_prompt == -1 and stepKind_after_prompt == 0:
                            noStep2wrongStep = noStep2wrongStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('noStep2wrongStep')
                            incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                            
                            
                        elif stepKind_before_prompt == 0 and stepKind_after_prompt == -1:
                            wrongStep2noStep = wrongStep2noStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('wrongStep2noStep')
                            incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                            
                        elif stepKind_before_prompt == 0 and stepKind_after_prompt == 1:
                            wrongStep2rightStep = wrongStep2rightStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('wrongStep2rightStep')
                            correctResponse_updateMatrix(step[i], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i], number_of_prompts_till_C_stops_correct_step_parent[i], number_of_prompts_till_C_stops_correct_step_robot[i])
                            
                        elif stepKind_before_prompt == 0 and stepKind_after_prompt == 0:
                            if attempted_step_before_prompt[i] != attempted_step_after_prompt[i]:
                                wrongStep2wrongStep_Diff = wrongStep2wrongStep_Diff + 1
                                
                                rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                                cc = colNames.index('wrongStep2wrongStep_Diff')
                                incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                                
                            elif attempted_step_before_prompt[i] == attempted_step_after_prompt[i]:
                                wrongStep2wrongStep_Same = wrongStep2wrongStep_Same + 1
                                
                                rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                                cc = colNames.index('wrongStep2wrongStep_Same')
                                incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                                
                        
                        elif stepKind_before_prompt == 1 and stepKind_after_prompt == -1:
                            rightStep2noStep = rightStep2noStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('rightStep2noStep')
                            incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                            
                        elif stepKind_before_prompt == 1 and stepKind_after_prompt == 1:
                            if attempted_step_after_prompt[i] in extendSteps and attempted_step_before_prompt[i] == attempted_step_after_prompt[i]:
                                rightStep2rightStep_extend = rightStep2rightStep_extend + 1
                            
                                rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                                cc = colNames.index('rightStep2rightStep_extend')
                                correctResponse_updateMatrix(step[i], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i], number_of_prompts_till_C_stops_correct_step_parent[i], number_of_prompts_till_C_stops_correct_step_robot[i])
                                
                            else:
                                rightStep2rightStep_nonExtend = rightStep2rightStep_nonExtend + 1
                            
                                rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                                cc = colNames.index('rightStep2rightStep_nonExtend')
                                correctResponse_updateMatrix(step[i], extendSteps, rr, cc, responseMatrix, colNames, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i], number_of_prompts_till_C_stops_correct_step_parent[i], number_of_prompts_till_C_stops_correct_step_robot[i])
                                
                        elif stepKind_before_prompt == 1 and stepKind_after_prompt == 0:
                            rightStep2wrongStep = rightStep2wrongStep + 1
                            
                            rr = rowNames.index(getRowName(R_verbal[i], R_gesture[i], P_verbal[i], P_gesture[i]))
                            cc = colNames.index('rightStep2wrongStep')
                            incorrectResponse_updateMatrix(rr, cc, responseMatrix, number_of_Prompts_till_C_executes_correct_step_parent[i], number_of_Prompts_till_C_executes_correct_step_robot[i])
                            
                        
                        else:
                            assert(False)
                            
                        assert (totalNPrompts == \
                                    noStep2noStep +\
                                    noStep2rightStep +\
                                    noStep2wrongStep +\
                                    rightStep2rightStep_nonExtend +\
                                    rightStep2rightStep_extend +\
                                    rightStep2wrongStep +\
                                    rightStep2noStep +\
                                    wrongStep2wrongStep_Diff +\
                                    wrongStep2wrongStep_Same +\
                                    wrongStep2rightStep +\
                                    wrongStep2noStep)
                                    
#                         assert(np.sum(responseMatrix[:,colNames.index('noStep2noStep')]) == noStep2noStep)
#                         assert(np.sum(responseMatrix[:,colNames.index('noStep2rightStep')]) == noStep2rightStep)
#                         assert(np.sum(responseMatrix[:,colNames.index('noStep2wrongStep')]) == noStep2wrongStep)
#                         assert(np.sum(responseMatrix[:,colNames.index('rightStep2rightStep_nonExtend')]) == rightStep2rightStep_nonExtend)
#                         assert(np.sum(responseMatrix[:,colNames.index('rightStep2rightStep_extend')]) == rightStep2rightStep_extend)
#                         assert(np.sum(responseMatrix[:,colNames.index('rightStep2wrongStep')]) == rightStep2wrongStep)
#                         assert(np.sum(responseMatrix[:,colNames.index('rightStep2noStep')]) == rightStep2noStep)
#                         assert(np.sum(responseMatrix[:,colNames.index('wrongStep2wrongStep_Diff')]) == wrongStep2wrongStep_Diff)
#                         assert(np.sum(responseMatrix[:,colNames.index('wrongStep2wrongStep_Same')]) == wrongStep2wrongStep_Same)
#                         assert(np.sum(responseMatrix[:,colNames.index('wrongStep2rightStep')]) == wrongStep2rightStep)
#                         assert(np.sum(responseMatrix[:,colNames.index('wrongStep2noStep')]) == wrongStep2noStep)
                        
                        ## P involvement level
                        if P_verbal[i] == 2: #P prompts for step
                            nPVerbals_prompts_step = nPVerbals_prompts_step + 1
                        if P_gesture[i] >= 5: #P's gesture touches C [nudge, guide arm, physically interene]
                            nPGestures_is_physical = nPGestures_is_physical + 1
                        
                    
                    ## C looks at P/R
                    if (P_R_PandR[i] == 1):
                        nPrompts_P = nPrompts_P + 1
                        if C_looks_at_P_R[i] == 1 or C_looks_at_P_R[i] == 3:
                            C_looks_P_given_P_prompt = C_looks_P_given_P_prompt + 1
                    elif P_R_PandR[i] == 2:
                        nPrompts_R = nPrompts_R + 1
                        if C_looks_at_P_R[i] == 2 or C_looks_at_P_R[i] == 3:
                            C_looks_R_given_R_prompt = C_looks_R_given_R_prompt + 1
                    elif P_R_PandR[i] == 3:
                        nPrompts_PnR = nPrompts_PnR + 1
                        if C_looks_at_P_R[i] == 1 or C_looks_at_P_R[i] == 3:
                            C_looks_P_given_PnR_prompt = C_looks_P_given_PnR_prompt + 1
                        elif C_looks_at_P_R[i] == 2 or C_looks_at_P_R[i] == 3:
                            C_looks_R_given_PnR_prompt = C_looks_R_given_PnR_prompt + 1
                    else:
                        assert(False)
                        
                    #note: totalNPrompts isn't necessarily == nPrompts_P + nPrompts_R + nPrompts_PnR, since we didn't count rewards and AG towards totalNPrompts
                    
                #end if P_R_PandR[i] > 0
                
                
                    
                ## Duration of C's responses
                #adds up all counts till C attempts correct step or the step is changed (must be after prompt):
                    #average time since prompt till C attempts correct step
                    #average number of prompts till C attempts correct step
                
                #only count once every time C attempts a [b]correct[\b] step (may or may not be after prompt):
                    #average number of seconds since C starts till C stops step before prompt (count soap, scrub + rinse, and dry separately)
                    #percentage of C's attempted correct steps that stops before prompt (include soap)
                    #average number of prompts since C attempts a correct step before C stops before prompt (exclude soap)
                    #average number of prompts since C starts soap before C stops
                    #average number of presses for soap
                
                
                # when C is prompted, mark down the 1st prompt time
                # then, when C executes the correct step, calculate the duration
                # reset the marked down 1st prompt time
                # 
                # when C is prompted, accumulate number of prompts before attempted to buffer
                # then, when C executes the correct step, calculate the sum of current number of prompts before attempt and the buffer
                # reset the buffer
                
                #if prompted
                if P_R_PandR[i]>0 and (P_verbal[i]>0 or P_gesture[i]>0 or R_verbal[i]>0 or R_gesture[i]>0) and time_1st_prompt: #time_1st_prompt needs to be non empty, hence "if time_1st_prompt:"
                    if promptStartTime == -1:
                        promptStartTime = time_1st_prompt[i]
                    nPromptsTillCAttemptsCorrectStep = nPromptsTillCAttemptsCorrectStep + max(number_of_Prompts_till_C_executes_correct_step_parent[i], 0) + max(number_of_Prompts_till_C_executes_correct_step_robot[i], 0)
                
                #if C executes correct step before or after prompt
                if isSameStep(attempted_step_before_prompt[i], step[i]) or isSameStep(attempted_step_after_prompt[i], step[i]) or attempted_step_is_correct_although_different_from_prompt[i] == 1:
                    if promptStartTime > 0:
                        durationTillCAttemptsCorrectStep = time_C_attempts_step[i] - promptStartTime
                        if durationTillCAttemptsCorrectStep > 0:
                            sumNumTimeCAttemptsCorrectStepAfterPrompted = sumNumTimeCAttemptsCorrectStepAfterPrompted + 1
                            sumDurationTillCAttemptsCorrectStep = sumDurationTillCAttemptsCorrectStep + durationTillCAttemptsCorrectStep
                            sumPromptsTillCAttemptsCorrectStep = sumPromptsTillCAttemptsCorrectStep + nPromptsTillCAttemptsCorrectStep
                        promptStartTime = -1
                        nPromptsTillCAttemptsCorrectStep = 0
                    
                    sumNumTimeCAttemptsCorrectStepOverall = sumNumTimeCAttemptsCorrectStepOverall + 1
                    
                    # possibilities:
                    # 	wrong step before prompt, correct step after
                    # 	no step before prompt, correct step after
                    # 	correct step before prompt, wrong step after
                    # 	correct step before prompt, no step after
                    # 	correct step before prompt, blank
                    # 	blank, correct step after
                    # 	correct step before prompt, correct step after => same
                    # 	===> look at the correct step when calculating duration of execution
                    #
                    #   correct step before, wrong step after \
                    #   wrong step before, correct step after
                    #   and still okay flag up
                    #   ===> this doesn't really matter most likely, just go with the step that's correct
                    # 	
                    # 	wrong step before, wrong step after \
                    # 	no step before, wrong step after \
                    # 	blank, wrong step after \
                    #   and still okay flag up
                    # 	===> look at the step after prompt when calculating duration of execution
                    # 	
                    # 	wrong step before, blank \
                    # 	wrong step before, no step after \
                    #   and still okay flag up
                    # 	===> look at the step before prompt when calculating duration of execution     
                    
                    #find the step C is executing:
                    #if there's a correct step, use that
                    #if still okay flag up, look at step after prompt if it's a wrong step and not a no step
                    #else look at step before
                    stepExecuted = -1
                    if isSameStep(attempted_step_before_prompt[i], step[i]) or isSameStep(attempted_step_after_prompt[i], step[i]):
                        stepExecuted = step[i]
                    elif attempted_step_is_correct_although_different_from_prompt[i] == 1:
                        if attempted_step_after_prompt[i] > 0:
                            stepExecuted = attempted_step_after_prompt[i]
                        else:
                            stepExecuted = attempted_step_before_prompt[i]
                    else:
                        assert (False)
                    assert (stepExecuted != -1)
                    
                    if C_stops_correct_step_before_next_prompt and C_stops_correct_step_before_next_prompt[i] == 1: #C_stops_correct_step_before_next_prompt needs to be non-empty
                        sumNumTimeCStopsBeforePrompt = sumNumTimeCStopsBeforePrompt + 1
                        
                        durationTillCStopsStep = time_C_stops_step[i] - time_C_attempts_step[i]
                        assert (durationTillCStopsStep >= 0)
                        
                        if stepExecuted == 3: #if C executes soap step
                            sumNumTimeCStopsSoap = sumNumTimeCStopsSoap + 1
                            sumDurationTillCStopsSoap = sumDurationTillCStopsSoap + durationTillCStopsStep
                            sumPromptsTillCStopsSoap = sumPromptsTillCStopsSoap + number_of_prompts_till_C_stops_correct_step_parent[i] + number_of_prompts_till_C_stops_correct_step_robot[i]
                            sumNumPressesTillCStopsSoap = sumNumPressesTillCStopsSoap + number_of_presses_till_C_stops_soap[i]
                        elif stepExecuted in [4, 5, 9]: #if C executes scrub / rinse / wet
                            sumNumTimeCStopsRinse = sumNumTimeCStopsRinse + 1
                            sumDurationTillCStopsRinse = sumDurationTillCStopsRinse + durationTillCStopsStep
                            sumPromptsTillCStopsRinse = sumPromptsTillCStopsRinse + number_of_prompts_till_C_stops_correct_step_parent[i] + number_of_prompts_till_C_stops_correct_step_robot[i]
                        elif stepExecuted == 7: #if C executes dry
                            sumNumTimeCStopsDry = sumNumTimeCStopsDry + 1
                            sumDurationTIllCStopsDry = sumDurationTIllCStopsDry + durationTillCStopsStep
                            sumPromptsTillCStopsDry = sumPromptsTillCStopsDry + number_of_prompts_till_C_stops_correct_step_parent[i] + number_of_prompts_till_C_stops_correct_step_robot[i]
                            
                            
                            
                
                ## Step Completion After Prompt
                #find an attempt after prompt
                if attempted_step_after_prompt[i] > 0 and attempted_step_after_prompt[i] != 8:
                    if attempted_step_after_prompt[i] in extendSteps:
                        
                        if (previousAttemptingStep_afterPrompt != attempted_step_after_prompt[i]) and (nContinuousAttemptedSteps_afterPrompt != 0):
                            
                            #todo: don't count cases where attempted_step_successfully_executed_after_prompt[i] == -1
                            
                            #we are only counting C's consecutive rinsing / drying as one success to be fair.
                            stepAttemptedCount_afterPrompt = stepAttemptedCount_afterPrompt + 1
                            stepCompletedCount_afterPrompt = stepCompletedCount_afterPrompt + 1. * nContinuousCompletedSteps_afterPrompt / nContinuousAttemptedSteps_afterPrompt
#                             #todo: debug
#                             print ('stepAttemptedCount_afterPrompt', stepAttemptedCount_afterPrompt)
#                             print ('stepCompletedCount_afterPrompt', stepCompletedCount_afterPrompt)
#                             print ('nContinuousCompletedSteps_afterPrompt', nContinuousCompletedSteps_afterPrompt)
#                             print ('nContinuousAttemptedSteps_afterPrompt', nContinuousAttemptedSteps_afterPrompt)
#                             print ('\n')
                            nContinuousAttemptedSteps_afterPrompt = 0
                            nContinuousCompletedSteps_afterPrompt = 0
                        previousAttemptingStep_afterPrompt = attempted_step_after_prompt[i]
                        nContinuousAttemptedSteps_afterPrompt = nContinuousAttemptedSteps_afterPrompt + 1
                        if attempted_step_successfully_executed_after_prompt[i] >= 1:
                            stepCompletedThisStep_overall = True
                            if P_gesture[i] < 7: #not because P physically intervened
                                nContinuousCompletedSteps_afterPrompt = nContinuousCompletedSteps_afterPrompt + 1
                    else:
                        if nContinuousAttemptedSteps_afterPrompt != 0:
                            stepAttemptedCount_afterPrompt = stepAttemptedCount_afterPrompt + 1
                            stepCompletedCount_afterPrompt = stepCompletedCount_afterPrompt + 1. * nContinuousCompletedSteps_afterPrompt / nContinuousAttemptedSteps_afterPrompt
                            nContinuousAttemptedSteps_afterPrompt = 0
                            nContinuousCompletedSteps_afterPrompt = 0
                        previousAttemptingStep_afterPrompt = attempted_step_after_prompt[i]
                        if attempted_step_before_prompt[i] != attempted_step_after_prompt[i]:
                            stepAttemptedCount_afterPrompt = stepAttemptedCount_afterPrompt + 1
                            if attempted_step_successfully_executed_after_prompt[i] >= 1:
                                stepCompletedThisStep_overall = True
                                if P_gesture[i] < 7: #not because P physically intervened
                                    stepCompletedCount_afterPrompt = stepCompletedCount_afterPrompt + 1
            
            
            ## Analysis Revision 2.0 - overall step completion count
#             #todo: debug
#             if correctStepCompleted_oneStep == False:
#                 print('not completed step idx', i)
#                 print('step', step[i])
            
            if correctStepCompleted_oneStep == True:
                correctStepCompletedCount_total = correctStepCompletedCount_total + 1
                if correctStepCompleted_withoutPhysicalPrompts_oneStep == True:
                    correctStepCompletedCount_withoutPhysicalPrompt_total = correctStepCompletedCount_withoutPhysicalPrompt_total + 1
                    if correctStepCompleted_withoutPPrompts_oneStep == True:
                        correctStepCompletedCount_withoutPPrompts_total = correctStepCompletedCount_withoutPPrompts_total + 1
                        if correctStepCompleted_withoutPnRPrompts_oneStep == True:
                            correctStepCompletedCount_withoutPnRPrompts_total = correctStepCompletedCount_withoutPnRPrompts_total + 1
            
            
            ## Step Completion After Prompt
            if nContinuousAttemptedSteps_afterPrompt != 0:
                #one last batch of conntinuousAttempt after exiting the for loop
                stepAttemptedCount_afterPrompt = stepAttemptedCount_afterPrompt + 1
                stepCompletedCount_afterPrompt = stepCompletedCount_afterPrompt + 1. * nContinuousCompletedSteps_afterPrompt / nContinuousAttemptedSteps_afterPrompt
                nContinuousAttemptedSteps_afterPrompt = 0
                nContinuousCompletedSteps_afterPrompt = 0
                
#             print ('stepAttemptedCount_afterPrompt', stepAttemptedCount_afterPrompt)
#             print ('stepCompletedCount_afterPrompt', stepCompletedCount_afterPrompt)
#             print ('\n')

            if not anyAttemptsThisStep_beforePrompt:
                stepNotAttempted_beforePrompt = stepNotAttempted_beforePrompt+1
            
            if stepCompletedThisStep_overall:
                stepCompletedCount_overall = stepCompletedCount_overall+1
            
            
            oneStep = []
            #end for i in oneStep
            
        if idx<len(step) and step[idx] > 1 and (step[idx] < 8 or step[idx] == 9): #we don't care about intro or all done step (and certainly not NA step)
            currentStep = step[idx]
            oneStep.append(idx)
        
        if idx<len(step):
            ## Analysis Revision 2.0 - number of prompts issued
            sumNumPromptsTotal_parent = sumNumPromptsTotal_parent + max(number_of_Prompts_till_C_executes_correct_step_parent[idx], 0) + max(number_of_prompts_till_C_stops_correct_step_parent[idx], 0)
            if P_gesture[idx] >= 5: #P gesture is physical
                sumNumPromptsPhysical_parent = sumNumPromptsPhysical_parent + max(number_of_Prompts_till_C_executes_correct_step_parent[idx], 0) #only count prompts before attempt, since usually physical gestures are not for continuing an extend step, so let's just ignore them
            sumNumPromptsTotal_robot = sumNumPromptsTotal_robot + max(number_of_Prompts_till_C_executes_correct_step_robot[idx], 0) + max(number_of_prompts_till_C_stops_correct_step_robot[idx], 0)
            
            ## Analysis Revision 2.0 - Engagement Level
            if C_smiles[idx] > 0:
                numCSmiles = numCSmiles + C_smiles[idx]
            if C_murmurs[idx] > 0:
                numCMurmurs = numCMurmurs + C_murmurs[idx]
            if C_distracted[idx] > 0:
                numCDistracted = numCDistracted + C_distracted[idx]
        
    
    #end for idx in range(0, len(step)+1)
    
    ## C attempts step before prompt
    stepAttemptedRate_beforePrompt = 1. * stepAttemptedCount_beforePrompt / (stepAttemptedCount_beforePrompt + stepNotAttempted_beforePrompt)
    stepCompletionRate_beforePrompt = np.NaN
    if stepAttemptedCount_beforePrompt > 0:
        stepCompletionRate_beforePrompt = 1. * stepCompletedCount_beforePrompt / stepAttemptedCount_beforePrompt
    
    ## C response types
#     print(\
#                 noStep2noStep ,\
#                 noStep2rightStep ,\
#                 noStep2wrongStep ,\
#                 rightStep2rightStep_nonExtend ,\
#                 rightStep2rightStep_extend ,\
#                 rightStep2wrongStep ,\
#                 rightStep2noStep ,\
#                 wrongStep2wrongStep_Diff ,\
#                 wrongStep2wrongStep_Same ,\
#                 wrongStep2rightStep ,\
#                 wrongStep2noStep)
#     print (totalNPrompts)
    
    assert (totalNPrompts == \
                noStep2noStep +\
                noStep2rightStep +\
                noStep2wrongStep +\
                rightStep2rightStep_nonExtend +\
                rightStep2rightStep_extend +\
                rightStep2wrongStep +\
                rightStep2noStep +\
                wrongStep2wrongStep_Diff +\
                wrongStep2wrongStep_Same +\
                wrongStep2rightStep +\
                wrongStep2noStep)

    #note: totalNPrompts isn't necessarily == nPrompts_P + nPrompts_R + nPrompts_PnR, since we didn't count rewards and AG towards totalNPrompts
    
#     no effects from prompt:
#         right2right_nonextend
#         wrong2wrong_same
#         nostep2nostep
#     stopped for reason:
#         right2nostep
#         wrong2nostep
#     special case:
#         right2wrong
#         wrong2wrong_diff
#         nostep2wrong
#     compliance:
#         right2right_extend
#         wrong2right
#         nostep2right
    no_effects_from_prompt_rate = 1.*(rightStep2rightStep_nonExtend + wrongStep2wrongStep_Same + noStep2noStep) / totalNPrompts
    stopped_for_no_reason_rate = 1.*(rightStep2noStep + wrongStep2noStep) / totalNPrompts
    special_case_rate = 1.*(rightStep2wrongStep + wrongStep2wrongStep_Diff + noStep2wrongStep) / totalNPrompts
    compliance_rate = 1.*(rightStep2rightStep_extend + wrongStep2rightStep + noStep2rightStep) / totalNPrompts
    
#     print ('no_effects_from_prompt_rate', no_effects_from_prompt_rate)
#     print ('stopped_for_no_reason_rate', stopped_for_no_reason_rate)
#     print ('special_case_rate', special_case_rate)
#     print ('compliance_rate', compliance_rate)
#     print ('\n')

    ## Analysis Revision 2.0 - Compliance Rates
    
#     print('responseMatrix', responseMatrix)
    
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
    
    compliance_hard_rate_overall = 1. * np.sum(np.array([1, 2, 3])) / np.array([0])
    
    responseMatrix_overall = np.sum(responseMatrix, 0)
    
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
    
    
#     print('sum_of_prompts_v', sum_of_prompts_v)
#     print('no_effects_from_prompt_rate_v', no_effects_from_prompt_rate_v)
#     print('stopped_for_no_reason_rate_v', stopped_for_no_reason_rate_v)
#     print('special_case_rate_v', special_case_rate_v)
#     print('compliance_rate_v', compliance_rate_v)
#     print('compliance_hard_rate_v', compliance_hard_rate_v)
#     
#     print('sum_of_prompts_overall', sum_of_prompts_overall)
#     print('no_effects_from_prompt_rate_overall', no_effects_from_prompt_rate_overall)
#     print('stopped_for_no_reason_rate_overall', stopped_for_no_reason_rate_overall)
#     print('special_case_rate_overall', special_case_rate_overall)
#     print('compliance_rate_overall', compliance_rate_overall)
#     print('compliance_hard_rate_overall', compliance_hard_rate_overall)
#     
#     print('\n')
    
    ## C looks at P/R
    C_looks_P_given_P_prompt_rate = np.NaN
    C_looks_R_given_R_prompt_rate = np.NaN
    C_looks_P_given_PnR_prompt_rate = np.NaN
    C_looks_R_given_PnR_prompt_rate = np.NaN
    if nPrompts_P > 0:
        C_looks_P_given_P_prompt_rate = 1.* C_looks_P_given_P_prompt / nPrompts_P
    if nPrompts_R > 0:
        C_looks_R_given_R_prompt_rate = 1.* C_looks_R_given_R_prompt / nPrompts_R
    if nPrompts_PnR > 0:
        C_looks_P_given_PnR_prompt_rate = 1.* C_looks_P_given_PnR_prompt / nPrompts_PnR
        C_looks_R_given_PnR_prompt_rate = 1.* C_looks_R_given_PnR_prompt / nPrompts_PnR

#     print ('C_looks_P_given_P_prompt_rate', C_looks_P_given_P_prompt_rate)
#     print ('C_looks_R_given_R_prompt_rate', C_looks_R_given_R_prompt_rate)
#     print ('C_looks_P_given_PnR_prompt_rate', C_looks_P_given_PnR_prompt_rate)
#     print ('C_looks_R_given_PnR_prompt_rate', C_looks_R_given_PnR_prompt_rate)
#     print ('\n')
    
    
    ## Duration of C's responses
    avgTimeDurationTillCAttemptsCorrectStep = np.NaN
    avgNoPromptsTillCAttemptsCorrectStep = np.NaN
    if sumNumTimeCAttemptsCorrectStepAfterPrompted > 0:
        avgTimeDurationTillCAttemptsCorrectStep = 1. * sumDurationTillCAttemptsCorrectStep / sumNumTimeCAttemptsCorrectStepAfterPrompted
        avgNoPromptsTillCAttemptsCorrectStep = 1. * sumPromptsTillCAttemptsCorrectStep / sumNumTimeCAttemptsCorrectStepAfterPrompted
    
    rateCStopsBeforePrompt = np.NaN
    if sumNumTimeCAttemptsCorrectStepOverall > 0:
        rateCStopsBeforePrompt = 1. * sumNumTimeCStopsBeforePrompt / sumNumTimeCAttemptsCorrectStepOverall
    
    avgTimeDurationTillCStopsSoap = np.NaN
    avgNoPromptsTillCStopsSoap = np.NaN
    avgNoPressesTillCStopSoap = np.NaN
    avgTimeDurationTillCStopsRinse = np.NaN
    avgTimeDurationTillCStopsDry = np.NaN
    avgNoPromptsTillCStopsRinse = np.NaN
    avgNoPromptsTillCStopsDry = np.NaN
    if sumNumTimeCStopsSoap > 0:
        avgTimeDurationTillCStopsSoap = 1. * sumDurationTillCStopsSoap / sumNumTimeCStopsSoap
        avgNoPromptsTillCStopsSoap = 1. * sumPromptsTillCStopsSoap / sumNumTimeCStopsSoap
        avgNoPressesTillCStopSoap = 1. * sumNumPressesTillCStopsSoap / sumNumTimeCStopsSoap
    if sumNumTimeCStopsRinse > 0:
        avgTimeDurationTillCStopsRinse = 1. * sumDurationTillCStopsRinse / sumNumTimeCStopsRinse
        avgNoPromptsTillCStopsRinse = 1. * sumPromptsTillCStopsRinse / sumNumTimeCStopsRinse
    if sumNumTimeCStopsDry > 0:
        avgTimeDurationTillCStopsDry = 1. * sumDurationTIllCStopsDry / sumNumTimeCStopsDry
        avgNoPromptsTillCStopsDry = 1. * sumPromptsTillCStopsDry / sumNumTimeCStopsDry
    
#     print ('avgTimeDurationTillCAttemptsCorrectStep', avgTimeDurationTillCAttemptsCorrectStep)
#     print ('avgNoPromptsTillCAttemptsCorrectStep', avgNoPromptsTillCAttemptsCorrectStep)
#     print ('rateCStopsBeforePrompt', rateCStopsBeforePrompt)
#     print ('avgTimeDurationTillCStopsSoap', avgTimeDurationTillCStopsSoap)
#     print ('avgNoPromptsTillCStopsSoap', avgNoPromptsTillCStopsSoap)
#     print ('avgNoPressesTillCStopSoap', avgNoPressesTillCStopSoap)
#     print ('avgTimeDurationTillCStopsRinse', avgTimeDurationTillCStopsRinse)
#     print ('avgTimeDurationTillCStopsDry', avgTimeDurationTillCStopsDry)
#     print ('avgNoPromptsTillCStopsRinse', avgNoPromptsTillCStopsRinse)
#     print ('avgNoPromptsTillCStopsDry', avgNoPromptsTillCStopsDry)
#     print ('\n')
    
    
    ## Step Completion After Prompt
    stepCompletionRate_afterPrompt = np.NaN
    if stepAttemptedCount_afterPrompt > 0:
        stepCompletionRate_afterPrompt = 1. * stepCompletedCount_afterPrompt / stepAttemptedCount_afterPrompt    
#     #todo: debug
#     print ('stepCompletionRate_afterPrompt', stepCompletionRate_afterPrompt)
#     print ('stepCompletedCount_afterPrompt', stepCompletedCount_afterPrompt)
#     print ('stepAttemptedCount_afterPrompt', stepAttemptedCount_afterPrompt)
    
#     print ('stepCompletionRate_afterPrompt', stepCompletionRate_afterPrompt)
#     print ('stepCompletedCount_afterPrompt', stepCompletedCount_afterPrompt)
#     print ('stepAttemptedCount_afterPrompt', stepAttemptedCount_afterPrompt)
#     print ('\n')
    
    stepCompletionRate_overall = 1. * stepCompletedCount_overall / nSteps
    
#     print ('stepAttemptedRate_beforePrompt', stepAttemptedRate_beforePrompt)
#     print ('stepCompletionRate_beforePrompt', stepCompletionRate_beforePrompt)
#     print ('stepCompletionRate_afterPrompt', stepCompletionRate_afterPrompt)
#     print ('stepCompletionRate_overall', stepCompletionRate_overall)
#     print ('\n')

    ## P involvement level
    assert (totalNPrompts >= nPGestures_is_physical + nPVerbals_prompts_step)
    percPVerbals_prompts_step = 1. * nPVerbals_prompts_step / totalNPrompts
    percPGestures_is_physical = 1. * nPGestures_is_physical / totalNPrompts
    percPPrompts_for_compliance = 1. * (totalNPrompts - nPGestures_is_physical - nPVerbals_prompts_step) / totalNPrompts
    
    
    ##calculated return values
    retDict = collections.OrderedDict()
    
    retDict['Step Attempted Rate - Before Prompt'] = stepAttemptedRate_beforePrompt
    retDict['Step Completion Rate - Before Prompt'] = stepCompletionRate_beforePrompt
    retDict['Step Completion Rate - After Prompt'] = stepCompletionRate_afterPrompt
    retDict['Step Completion Rate - Overall'] = stepCompletionRate_overall
    
    retDict['Number of Incomplete Steps'] = nSteps - correctStepCompletedCount_total
    retDict['Number of Incomplete Steps - Before P Phyiscal'] = nSteps - correctStepCompletedCount_withoutPhysicalPrompt_total
    retDict['Number of Incomplete Steps - Before P'] = nSteps - correctStepCompletedCount_withoutPPrompts_total
    retDict['Number of Incomplete Steps - Before P or R'] = nSteps - correctStepCompletedCount_withoutPnRPrompts_total
    
    retDict['Looking at Parent Rate Given Parent Prompted'] = C_looks_P_given_P_prompt_rate
    retDict['Looking at Robot Rate Given Robot Prompted'] = C_looks_R_given_R_prompt_rate
    retDict['Looking at Parent Rate Given Both Prompted'] = C_looks_P_given_PnR_prompt_rate
    retDict['Looking at Robot Rate Given Both Prompted'] = C_looks_R_given_PnR_prompt_rate
    
    retDict['Total Number of Times Child Smiles'] = numCSmiles
    retDict['Total Number of Times Child Murmurs'] = numCMurmurs
    retDict['Total Number of Times Child Is Distracted'] = numCDistracted
    
    retDict['Average Duration Till Attempting Correct Step'] = avgTimeDurationTillCAttemptsCorrectStep
    retDict['Average Number of Prompts Till Attempting Correct Step'] = avgNoPromptsTillCAttemptsCorrectStep
    retDict['Stopping Step Before Prompted Rate'] = rateCStopsBeforePrompt
    retDict['Average Duration Till Stopping Step - Soap'] = avgTimeDurationTillCStopsSoap
    retDict['Average Number of Prompts Till Stopping Step - Soap'] = avgNoPromptsTillCStopsSoap
    retDict['Average Number of Presses Till Stopping Step - Soap'] = avgNoPressesTillCStopSoap
    retDict['Average Duration Till Stopping Step - Rinse'] = avgTimeDurationTillCStopsRinse
    retDict['Average Duration Till Stopping Step - Dry'] = avgTimeDurationTillCStopsDry
    retDict['Average Number of Prompts Till Stopping Step - Rinse'] = avgNoPromptsTillCStopsRinse
    retDict['Average Number of Prompts Till Stopping Step - Dry'] = avgNoPromptsTillCStopsDry
    
    retDict['Total Number of Parent Prompts'] = sumNumPromptsTotal_parent
    retDict['Total Number of Parent Prompts - Physical'] = sumNumPromptsPhysical_parent
    retDict['Total Number of Parent Prompts - Non-physical'] = sumNumPromptsTotal_parent - sumNumPromptsPhysical_parent
    retDict['Total Number of Robot Prompts'] = sumNumPromptsTotal_robot
    retDict['Total Number of Prompts - Robot and Parent'] = sumNumPromptsTotal_parent + sumNumPromptsTotal_robot
    
    retDict['Not Affected By Prompt Rate'] = no_effects_from_prompt_rate
    retDict['Stopped For No Reason Rate'] = stopped_for_no_reason_rate
    retDict['Confused By Prompt Rate'] = special_case_rate
    retDict['Compliance Rate'] = compliance_rate
    
    for ii, value in enumerate(no_effects_from_prompt_rate_v):
        retDict['Not Affected By Prompt Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(stopped_for_no_reason_rate_v):
        retDict['Stopped For No Reason Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(special_case_rate_v):
        retDict['Confused By Prompt Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(compliance_rate_v):
        retDict['Compliance Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(compliance_hard_rate_v):
        retDict['Hard Compliance Rate - ' + rowNames[ii]] = value
    
    retDict['Not Affected By Prompt Rate - Overall'] = no_effects_from_prompt_rate_overall
    retDict['Stopped For No Reason Rate - Overall'] = stopped_for_no_reason_rate_overall
    retDict['Confused By Prompt Rate - Overall'] = special_case_rate_overall
    retDict['Compliance Rate - Overall'] = compliance_rate_overall
    retDict['Hard Compliance Rate - Overall'] = compliance_hard_rate_overall
    
    retDict['Percentage of Parent Prompts on Compliance'] = percPPrompts_for_compliance
    retDict['Percentage of Parent Prompts on Step Instructions'] = percPVerbals_prompts_step
    retDict['Percentage of Parent Prompts that Is Physical'] = percPGestures_is_physical
    
    retDict['Parent Involvement'] = parent_involvement
    
    retDict['Number of Complete Steps - With P and R'] = correctStepCompletedCount_total
    retDict['Number of Complete Steps - With R'] = correctStepCompletedCount_withoutPPrompts_total
    retDict['Number of Complete Steps - Without P or R'] = correctStepCompletedCount_withoutPnRPrompts_total
    
    ##sample sizes
    smplSzDict = collections.OrderedDict()
    
    smplSzDict['Step Attempted Rate - Before Prompt'] = (stepAttemptedCount_beforePrompt + stepNotAttempted_beforePrompt)
    smplSzDict['Step Completion Rate - Before Prompt'] = stepAttemptedCount_beforePrompt
    smplSzDict['Step Completion Rate - After Prompt'] = stepAttemptedCount_afterPrompt
    smplSzDict['Step Completion Rate - Overall'] = nSteps
    
    smplSzDict['Number of Incomplete Steps'] = 1
    smplSzDict['Number of Incomplete Steps - Before P Phyiscal'] = 1
    smplSzDict['Number of Incomplete Steps - Before P'] = 1
    smplSzDict['Number of Incomplete Steps - Before P or R'] = 1
    
    smplSzDict['Looking at Parent Rate Given Parent Prompted'] = nPrompts_P
    smplSzDict['Looking at Robot Rate Given Robot Prompted'] = nPrompts_R
    smplSzDict['Looking at Parent Rate Given Both Prompted'] = nPrompts_PnR
    smplSzDict['Looking at Robot Rate Given Both Prompted'] = nPrompts_PnR
    
    smplSzDict['Total Number of Times Child Smiles'] = 1
    smplSzDict['Total Number of Times Child Murmurs'] = 1
    smplSzDict['Total Number of Times Child Is Distracted'] = 1
    
    smplSzDict['Average Duration Till Attempting Correct Step'] = sumNumTimeCAttemptsCorrectStepAfterPrompted
    smplSzDict['Average Number of Prompts Till Attempting Correct Step'] = sumNumTimeCAttemptsCorrectStepAfterPrompted
    smplSzDict['Stopping Step Before Prompted Rate'] = sumNumTimeCAttemptsCorrectStepOverall
    smplSzDict['Average Duration Till Stopping Step - Soap'] = sumNumTimeCStopsSoap
    smplSzDict['Average Number of Prompts Till Stopping Step - Soap'] = sumNumTimeCStopsSoap
    smplSzDict['Average Number of Presses Till Stopping Step - Soap'] = sumNumTimeCStopsSoap
    smplSzDict['Average Duration Till Stopping Step - Rinse'] = sumNumTimeCStopsRinse
    smplSzDict['Average Duration Till Stopping Step - Dry'] = sumNumTimeCStopsDry
    smplSzDict['Average Number of Prompts Till Stopping Step - Rinse'] = sumNumTimeCStopsRinse
    smplSzDict['Average Number of Prompts Till Stopping Step - Dry'] = sumNumTimeCStopsDry
    
    smplSzDict['Total Number of Parent Prompts'] = 1
    smplSzDict['Total Number of Parent Prompts - Physical'] = 1
    smplSzDict['Total Number of Parent Prompts - Non-physical'] = 1
    smplSzDict['Total Number of Robot Prompts'] = 1
    smplSzDict['Total Number of Prompts - Robot and Parent'] = 1
    
    smplSzDict['Not Affected By Prompt Rate'] = totalNPrompts
    smplSzDict['Stopped For No Reason Rate'] = totalNPrompts
    smplSzDict['Confused By Prompt Rate'] = totalNPrompts
    smplSzDict['Compliance Rate'] = totalNPrompts
    
    for ii, value in enumerate(no_effects_from_prompt_rate_v_SmplSz):
        smplSzDict['Not Affected By Prompt Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(stopped_for_no_reason_rate_v_SmplSz):
        smplSzDict['Stopped For No Reason Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(special_case_rate_v_SmplSz):
        smplSzDict['Confused By Prompt Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(compliance_rate_v_SmplSz):
        smplSzDict['Compliance Rate - ' + rowNames[ii]] = value
    for ii, value in enumerate(compliance_hard_rate_v_SmplSz):
        smplSzDict['Hard Compliance Rate - ' + rowNames[ii]] = value
    
    smplSzDict['Not Affected By Prompt Rate - Overall'] = no_effects_from_prompt_rate_overall_SmplSz
    smplSzDict['Stopped For No Reason Rate - Overall'] = stopped_for_no_reason_rate_overall_SmplSz
    smplSzDict['Confused By Prompt Rate - Overall'] = special_case_rate_overall_SmplSz
    smplSzDict['Compliance Rate - Overall'] = compliance_rate_overall_SmplSz
    smplSzDict['Hard Compliance Rate - Overall'] = compliance_hard_rate_overall_SmplSz
    
    smplSzDict['Percentage of Parent Prompts on Compliance'] = totalNPrompts
    smplSzDict['Percentage of Parent Prompts on Step Instructions'] = totalNPrompts
    smplSzDict['Percentage of Parent Prompts that Is Physical'] = totalNPrompts
    
    smplSzDict['Parent Involvement'] = 1
    
    smplSzDict['Number of Complete Steps - With P and R'] = 1
    smplSzDict['Number of Complete Steps - With R'] = 1
    smplSzDict['Number of Complete Steps - Without P or R'] = 1
    
    return [retDict, smplSzDict]
    

## script start

#load the files
directory = 'C:\\Users\\David-PC\\SkyDrive\\thesis report\\data analysis\\analysis after annotation\\'
names_t1 = ['t1-s1.csv', 't1-s2.csv', 't1-s3.csv', 't1-s4.csv', 't1-s5.csv', 't1-s6.csv', 't1-s7.csv', 't1-s8.csv']
names_t2 = ['t2-s1.csv', 't2-s2.csv', 't2-s3.csv', 't2-s4.csv', 't2-s5.csv', 't2-s6.csv', 't2-s7.csv', 't2-s8.csv']
names_t3 = ['t3-s1.csv', 't3-s2.csv', 't3-s3.csv', 't3-s4.csv', 't3-s5.csv', 't3-s6.csv']
names_t4 = ['t4-s1.csv', 't4-s2.csv', 't4-s3.csv', 't4-s4.csv', 't4-s5.csv', 't4-s6.csv', 't4-s7.csv', 't4-s8.csv', 't4-s9.csv']
names_t5 = ['t5-s1.csv', 't5-s2.csv', 't5-s3.csv', 't5-s4.csv', 't5-s5.csv', 't5-s6.csv', 't5-s7.csv', 't5-s8.csv', 't5-s9.csv']
names_t6 = ['t6-s1.csv', 't6-s2.csv', 't6-s3.csv', 't6-s4.csv', 't6-s5.csv', 't6-s6.csv', 't6-s7.csv', 't6-s8.csv', 't6-s9.csv', 't6-s10.csv']
names_all = [names_t1, names_t2, names_t3, names_t4, names_t5, names_t6]
# names_all = [['t5-s8.csv']] #todo: debug
names_all_1D = []
for names in names_all:
    for name in names:
        names_all_1D.append(name)

#getting the results
resultsDict = collections.OrderedDict()
resultsSmplSzDict = collections.OrderedDict()
currRowIdx = 0
for i, names in enumerate(names_all):
    for j, filename in enumerate(names):
        print ('filename', filename)
        result, resultSmplSz = calcStepCompletionRate(directory + filename)
        
        if len(resultsDict.keys()) == 0: #need to initialize the dict and create lists
        
            resultsDict = collections.OrderedDict.fromkeys(result.keys())
            assert(cmp(result.keys(), resultSmplSz.keys() == 0))
            resultsSmplSzDict = collections.OrderedDict.fromkeys(result.keys())
            
            #calculate total number of points (sessions) and set resultsDict as a dict arrays of that size
            numPoints = 0
            for names in names_all:
                numPoints = numPoints + len(names)
            for key1 in resultsDict:
                resultsDict[key1] = np.zeros(numPoints)
                resultsSmplSzDict[key1] = np.zeros(numPoints, dtype=int)
#                 resultsDict[key1] = copy.deepcopy(names_all) #just using the strings in names_all list as a place holder, will fill in the elements with calculated values in result
#                 resultsSmplSzDict[key1] = copy.deepcopy(names_all) 
        for key2 in result:
            if (currRowIdx + j) == 50:
                blah = 1
            resultsDict[key2][currRowIdx + j] = result[key2]
            resultsSmplSzDict[key2][currRowIdx + j] = resultSmplSz[key2]
#             resultsDict[key2][i][j] = result[key2]
#             resultsSmplSzDict[key2][i][j] = resultSmplSz[key2]
    currRowIdx = currRowIdx + len(names_all[i])



## result visualization

# #phase segments
# parentInvolvementSegs = [16, 6, 2, 1, 1, 5, 1, 2, 3, 2, 1, 2, 1, 2, 5]
# parentInvolvementLabels = [5, 0, 1, 3, 1, 2, 1, 4, 2, 4, 1, 2, 1, 4, 1]
# parentInvolvementLabelsStr = ['R Alone', 'R Alone Rep', 'R + P Compliance', 'R + P Step', 'R + P Behind', 'P Alone']
# parentInvolvementLabelsFull = []
# assert(len(parentInvolvementSegs) == len(parentInvolvementLabels))
# for ii, seg in enumerate(parentInvolvementSegs):
#     for jj in range(seg):
#         parentInvolvementLabelsFull.append(parentInvolvementLabels[ii])
# parentInvolvementIndx = [] #[0, 16, 24, 25, 26, 31, 32, 34, 37, 39, 40, 42, 43, 45, 50]
# lastSeg = 0
# for seg in parentInvolvementSegs:
#     if not parentInvolvementIndx: #list empty
#         parentInvolvementIndx.append(0)
#     else:
#         parentInvolvementIndx.append(lastSeg + parentInvolvementIndx[-1])
#     lastSeg = seg
# parentInvolvementIndx.append(lastSeg + parentInvolvementIndx[-1])

# segColors = ['r', 'm', 'g', 'k', '#FFFF66', 'b']
# assert(len(segColors) == len(np.unique(parentInvolvementLabels)))
# segShapes = ['o', 'o', 'v', 's', '*', 'd']
# assert(len(segShapes) == len(segColors))
# 
# plt.figure(1)
# timeAxis = np.arange(1, len(resultsDict[resultsDict.keys()[0]])+1)
# for i, key3 in enumerate(resultsDict):
#     plt.clf()
#     for segIdx, segColor in enumerate(segColors):
#         indices = np.array(parentInvolvementLabelsFull)==segIdx
#         plt.plot(timeAxis[indices], resultsDict[key3][indices], color=segColors[segIdx], marker=segShapes[segIdx], linestyle='None', markersize=10, label=parentInvolvementLabelsStr[segIdx])
#     #annotate the points
# #     for ii, result in enumerate(resultsDict[key3]):
# #         plt.annotate(str(resultsSmplSzDict[key3][ii]), xy=(ii+1, result), color=('#99FF33'), size=9) #sample size
#     #title, axes names, and legends
# #     plt.title(key3)
#     plt.xlabel('Trial Number')
#     plt.ylabel(key3)
#     plt.legend(loc='best', shadow=True, markerscale=0.5, numpoints=1, fontsize=9)
#     #x,y limits
#     plt.xlim(0.5, len(timeAxis)+1)
#     plt.ylim(ymin=-0.1)
#     if 'Rate' in key3 or 'Percentage' in key3:
#         plt.ylim(ymax=1.1)
# #         plt.ylim(-1, 1) #todo: debug
#     if 'Number of Incomplete Steps' in key3:
#         plt.ylim(ymax=7.1)
#     plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ') + '.png') #no spaces in names for latex use
# #     plt.savefig(directory + 'figures\\' + str(i) + ' ' + key3 + '_nolegend'+ '.png')



#phase segments
phaseSegs = [16, 8, 21, 5]
phaseLabel = ['A', 'B', 'C', 'B']

plt.figure(1)
timeAxis = np.arange(1, len(resultsDict[resultsDict.keys()[0]])+1)

for i, key3 in enumerate(resultsDict):
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
        
        phaseEndIndx = newPhaseEndIndx
        
#     for segIdx, segColor in enumerate(segColors):
#         indices = np.array(parentInvolvementLabelsFull)==segIdx
#         plt.plot(timeAxis[indices], resultsDict[key3][indices], color=segColors[segIdx], marker=segShapes[segIdx], linestyle='None', markersize=10, label=parentInvolvementLabelsStr[segIdx])
#     #sample size
#     for ii, result in enumerate(resultsDict[key3]):
#         plt.annotate(str(resultsSmplSzDict[key3][ii]), xy=(ii+1, result), color=('#99FF33'), size=9)
    
    
    #title, axes names, and legends
#     plt.title(key3)
    plt.xlabel('Trial Number')
    plt.ylabel(key3)
#     plt.legend(loc='best', shadow=True, markerscale=0.5, numpoints=1, fontsize=9)
    #x,y limits
    plt.xlim(0.5, len(timeAxis)+1)
    plt.ylim(ymin=-0.1)#make sure points don't touch boundary
    if 'Rate' in key3 or 'Percentage' in key3:
        plt.ylim(ymax=1.0)
#         plt.ylim(-1, 1) #todo: debug
    if 'Number of Incomplete Steps' in key3:
        plt.ylim(ymax=5)
    if 'Number of Complete Steps' in key3:
        plt.ylim(ymax=7)
    plt.ylim(ymax=plt.ylim()[1]+0.1)#make sure points don't touch boundary
    # annotate phases
    phaseEndIndx = 0
    for phaseIdx, phaseLen in enumerate(phaseSegs):
        newPhaseEndIndx = phaseEndIndx + phaseLen
        plt.annotate(phaseLabel[phaseIdx], xy=(0.5*(phaseEndIndx+newPhaseEndIndx), 0.95*plt.ylim()[1]+0.05*plt.ylim()[0]), color='k', size=20)
        phaseEndIndx = newPhaseEndIndx
#     plt.show()
    plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ') + '.eps') #no spaces in names for latex use
    plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ') + '.png') #no spaces in names for latex use
#     plt.savefig(directory + 'figures\\' + str(i) + ' ' + key3 + '_nolegend'+ '.png')


## plot robot alone trials only for compliance rate and not affected by prompt rate
keysToPlot = ['Compliance Rate - Overall', 'Hard Compliance Rate - Overall', 'Not Affected By Prompt Rate - Overall', 'Compliance Rate - R1Pv0g0', 'Hard Compliance Rate - R1Pv0g0']
for i, key3 in enumerate(resultsDict):
    if key3 in keysToPlot:
        plt.clf()
        plt.rc("font", size=18)
        phaseEndIndx = 0
        for phaseIdx, phaseLen in enumerate(phaseSegs):
            #form the indices of items of the phase
            newPhaseEndIndx = phaseEndIndx + phaseLen
            indices = range(phaseEndIndx, newPhaseEndIndx)
            
            #plot robot alone only
            robotAloneOnlyIndices = resultsDict['Parent Involvement'][indices]==0
            plt.plot(timeAxis[indices][robotAloneOnlyIndices], resultsDict[key3][indices][robotAloneOnlyIndices], color='k', marker='.', linestyle='-', linewidth=0.8, markersize=10)
            #draw vertical phase lines
            if phaseIdx != len(phaseSegs)-1: #don't draw line for the last seg
                plt.axvline(newPhaseEndIndx + 0.5, color='k')
            
            phaseEndIndx = newPhaseEndIndx
        
        #title, axes names, and legends
    #     plt.title(key3)
        plt.xlabel('Trial Number')
        plt.ylabel(key3)
    #     plt.legend(loc='best', shadow=True, markerscale=0.5, numpoints=1, fontsize=9)
        #x,y limits
        plt.xlim(0.5, len(timeAxis)+1)
        plt.ylim(ymin=-0.1)#make sure points don't touch boundary
        if 'Rate' in key3 or 'Percentage' in key3:
            plt.ylim(ymax=1.0)
    #         plt.ylim(-1, 1) #todo: debug
        if 'Number of Incomplete Steps' in key3:
            plt.ylim(ymax=5)
        plt.ylim(ymax=plt.ylim()[1]+0.1)#make sure points don't touch boundary
        # annotate phases
        phaseEndIndx = 0
        for phaseIdx, phaseLen in enumerate(phaseSegs):
            newPhaseEndIndx = phaseEndIndx + phaseLen
            plt.annotate(phaseLabel[phaseIdx], xy=(0.5*(phaseEndIndx+newPhaseEndIndx), 0.95*plt.ylim()[1]+0.05*plt.ylim()[0]), color='k', size=20)
            phaseEndIndx = newPhaseEndIndx
        plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ') + '_robotAloneOnly' + '.eps') #no spaces in names for latex use
        plt.savefig(directory + 'figures\\' + str(i) + key3.translate(None, ' ') + '_robotAloneOnly' + '.png') #no spaces in names for latex use
