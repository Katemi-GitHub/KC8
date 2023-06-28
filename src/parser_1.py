# Opcodes
INS_ADD =   "01"
INS_SUB =   "02"
INS_AND =   "03"
INS_OR =    "04"
INS_XOR =   "05"
INS_NOT =   "06"
INS_MOV =   "07"
INS_LOAD =  "08"
INS_STORE = "09"
INS_INC =   "0A"
INS_DEC =   "0B"
INS_JMP =   "0C"
INS_JZ =    "0D"
INS_JNZ =   "0E"
INS_CMP =   "0F"
INS_SHL =   "10"
INS_SHR =   "11"
INS_CALL =  "12"
INS_RET =   "13"
INS_LCD =   "14"
INS_CLR =   "15"
INS_HLT =   "16"
INS_NOP =   "00"

# Registers
REG_A = "01"
REG_I = "02"
REG_O = "03"
REG_X = "04"
REG_Y = "05"

def parse_instruction( line ):
    token = line.split( " " )
    opcode = None
    r1 = None
    r2 = None

    if ( token[0] == "ADD" ):
        opcode = INS_ADD
    elif ( token[0] == "SUB" ):
        opcode = INS_SUB
    elif ( token[0] == "AND" ):
        opcode = INS_AND
    elif ( token[0] == "OR" ):
        opcode = INS_OR
    elif ( token[0] == "XOR" ):
        opcode = INS_XOR
    elif ( token[0] == "NOT" ):
        opcode = INS_NOT
    elif ( token[0] == "MOV" ):
        opcode = INS_MOV
    elif ( token[0] == "LOAD" ):
        opcode = INS_LOAD
    elif ( token[0] == "STORE" ):
        opcode = INS_STORE
    elif ( token[0] == "INC" ):
        opcode = INS_INC
    elif ( token[0] == "DEC" ):
        opcode = INS_DEC
    elif ( token[0] == "JUMP" ):
        opcode = INS_JMP
    elif ( token[0] == "JZ" ):
        opcode = INS_JZ
    elif ( token[0] == "JNZ" ):
        opcode = INS_JNZ
    elif ( token[0] == "CMP" ):
        opcode = INS_CMP
    elif ( token[0] == "SHL" ):
        opcode = INS_SHL
    elif ( token[0] == "SHR" ):
        opcode = INS_SHR
    elif ( token[0] == "CALL" ):
        opcode = INS_CALL
    elif ( token[0] == "RETURN" ):
        opcode = INS_RET
    elif ( token[0] == "LCD" ):
        opcode = INS_LCD
    elif ( token[0] == "HALT" ):
        opcode = INS_HLT
    elif ( token[0] == "NOP" ):
        opcode = INS_NOP
    elif ( token[0] == "CLR" ):
        opcode = INS_CLR
    
    if ( len(token) > 2 ):
        if ( token[2][0] == "#" ):
            if ( token[2][1:] == "A" ):
                r2 = REG_A
            if ( token[2][1:] == "I" ):
                r2 = REG_I
            if ( token[2][1:] == "O" ):
                r2 = REG_O
            if ( token[2][1:] == "X" ):
                r2 = REG_X
            if ( token[2][1:] == "Y" ):
                r2 = REG_Y
        elif ( token[2][0] == "&" ):
            r2 = token[2][1:]
    else:
        r2 = "00"
    
    if ( token[1][0] == "#" ):
        if ( token[1][1:] == "A" ):
            r1 = REG_A
        if ( token[1][1:] == "I" ):
            r1 = REG_I
        if ( token[1][1:] == "O" ):
            r1 = REG_O
        if ( token[1][1:] == "X" ):
            r1 = REG_X
        if ( token[1][1:] == "Y" ):
            r1 = REG_Y
    elif ( token[1][0] == "$" ):
        r1 = token[1][1:3]
        r2 = token[1][3:5]
    elif ( token[1][0] == "&" ):
        r1 = token[1][1:]
    
    return opcode, r1, r2

file_path = "src/assembly.txt"
with open(file_path, 'r') as file:
    input_text = file.read()

lines = input_text.strip().split('\n')
output_lines = []
for line in lines:
    opcode, register, data = parse_instruction(line)
    output_lines.append(f"{opcode} {register} {data}")

output_file = "src/parsed_rom.txt"
with open(output_file, "w") as f:
    for i in range(0, len(output_lines), 5):
        chunk = output_lines[i:i + 5]
        line = ' '.join(chunk)
        f.write(line + '\n')