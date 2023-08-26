def stripDigit(input_string):
    digits = 0
    for char in input_string:
        if char.isdigit():
            digits = digits * 10 + int(char)
    return digits


# Define maximum number of titles and lines per title
MAX_TITLES = 20
MAX_Processes = 50

# Initialize arrays to store titles and their data
titles = [''] * MAX_TITLES
# data_arrays = [[[] for _ in range(MAX_TITLES)] for group in x]
data_arrays = [[[] for _ in range(5)] for _ in range(MAX_TITLES)]

# Read data from the file
with open('input.txt', 'r') as file:
    lines = file.readlines()

# Process the lines and populate arrays
title_index = -1
process_index = 0
titleN = 0
for line in lines:
    line = line.strip()
    if line == '#':
        titleN = 1
        continue
    elif titleN == 1 and line[0] != ' ':
        title_index += 1
        titles[title_index] = line
        titleN = 0
        process_index = 0
    else:
        words = line.split()
        print(f"titles: {words[0]}")
        words[0] = stripDigit(words[0])
        data_arrays[title_index][process_index] = words
        process_index += 1
print(f"titles: {titles}")
print(data_arrays)


