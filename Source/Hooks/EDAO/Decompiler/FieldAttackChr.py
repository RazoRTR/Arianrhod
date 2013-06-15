from Assembler2 import *
from EDAOBase import *

def GetOpCode(fs):
    return fs.byte()

def WriteOpCode(fs, op):
    return fs.wbyte(op)

edao_fa_op_table = InstructionTable(GetOpCode, WriteOpCode, DefaultGetLabelName, CODE_PAGE)

InstructionNames = {}

InstructionNames[0x00] = 'Return'
InstructionNames[0x01] = 'SetChrSubChip'
InstructionNames[0x02] = 'Sleep'
InstructionNames[0x03] = 'PlayEffect'
InstructionNames[0x04] = 'Sound'
InstructionNames[0x05] = 'Voice'
InstructionNames[0x06] = 'FA_06'
InstructionNames[0x07] = 'FA_07'
InstructionNames[0x08] = 'FA_08'
InstructionNames[0x09] = 'FA_09'
InstructionNames[0x0A] = 'BlurSwitch'
InstructionNames[0x0B] = 'FA_0B'
InstructionNames[0x0C] = 'FA_0C'
InstructionNames[0x0D] = 'FA_0D'


for op, name in InstructionNames.items():
    expr = '%s = 0x%08X' % (name, op)
    exec(expr)

class EDAOFAOpTableEntry(InstructionTableEntry):
    def __init__(self, op, name = '', operand = NO_OPERAND, flags = 0, handler = None):
        super().__init__(op, name, operand, flags, handler)

def inst(op, operand = NO_OPERAND, flags = 0, handler = None):
    return EDAOFAOpTableEntry(op, InstructionNames[op], operand, flags, handler)


edao_fa_op_list = \
[
    inst(Return,             NO_OPERAND,             INSTRUCTION_END_BLOCK),
    inst(SetChrSubChip,     'C'),
    inst(Sleep,             'H'),
    inst(PlayEffect,        'WHHHHHB'),
    inst(Sound,             'H'),
    inst(Voice,             'HHHH'),
    inst(FA_06,             NO_OPERAND),
    inst(FA_07,             NO_OPERAND),
    inst(FA_08,             'h'),
    inst(FA_09,             'HH'),
    inst(BlurSwitch,        'HH'),
    inst(FA_0B,             'H'),
    inst(FA_0C,             'WW'),
    inst(FA_0D,             NO_OPERAND),
]


for op in edao_fa_op_list:
    edao_fa_op_table[op.OpCode] = op
    op.Container = edao_fa_op_table


class FieldAttackFileInfo:

    def __init__(self):
        self.HeaderSize     = 0
        self.ChipFile       = ChipFileIndex()
        self.EffectName     = ''
        self.CodeBlock      = None

    def open(self, buf):
        if type(buf) == str:
            buf = open(buf, 'rb').read()

        fs = BytesStream()
        fs.openmem(buf)

        self.HeaderSize = fs.ushort()
        self.ChipFile   = ChipFileIndex(fs.ulong())
        self.EffectName = fs.astr()

        fs.seek(self.HeaderSize)

        self.CodeBlock = Disassembler(edao_fa_op_table).DisasmBlock(fs)

    def SaveToFile(self, filename):

        lines = []

        lines.append('from %s import *' % os.path.splitext(os.path.basename(__file__))[0])
        lines.append('')

        name = os.path.splitext(os.path.basename(filename))[0]
        name = os.path.splitext(name)[0]

        lines.append('CreateFieldAttack("%s", "%s", "%s")' % (name + '._bn', self.ChipFile.Name(), self.EffectName))
        lines.append('')

        LabelMap = { self.CodeBlock.Offset : '' }

        blocks = [Disassembler(edao_fa_op_table).FormatCodeBlock(self.CodeBlock, LabelMap)]
        for block in blocks:
            lines += block

        txt = '\r\n'.join(lines)

        lines = txt.replace('\r\n', '\n').replace('\r', '\n').split('\n')

        for i in range(2, len(lines)):
            if lines[i] != '':
                lines[i] = '    %s' % lines[i]

        lines.insert(2, 'def main():')
        lines.append('TryInvoke(main)')
        lines.append('')

        fs = open(filename, 'wb')
        fs.write(''.encode('utf_8_sig'))
        fs.write('\r\n'.join(lines).encode('UTF8'))


if __name__ == '__main__':
    def main():
        for f in sys.argv[1:]:
            asdat = FieldAttackFileInfo()
            asdat.open(f)
            asdat.SaveToFile(f + '.py')

    TryInvoke(main)



############################################################################################
# support functions
############################################################################################

fafile = None

def label(*args):
    pass

def CreateFieldAttack(FileName, ChipFile, EffectName):
    EffectName = EffectName.encode(CODE_PAGE) + b'\x00'

    global fafile

    fafile = BytesStream()
    fafile.open(FileName, 'wb')
    hdrsize = 2 + 4 + len(EffectName)

    fafile.wushort(hdrsize)
    fafile.wulong(ChipFileIndex(ChipFile).Index())
    fafile.write(EffectName)


for op, inst in edao_fa_op_table.items():

    func = []
    func.append('def %s(*args):' % inst.OpName)
    func.append('    return OpCodeHandler(0x%02X, args)' % inst.OpCode)
    func.append('')

    exec('\r\n'.join(func))

    opx = 'FA_%02X' % inst.OpCode

    if inst.OpName != opx:
        func[0] = 'def %s(*args):' % opx
        exec('\r\n'.join(func))


def DefaultOpCodeHandler(data):
    entry   = data.TableEntry
    fs      = data.FileStream
    inst    = data.Instruction
    oprs    = inst.OperandFormat
    values  = data.Arguments

    entry.Container.WriteOpCode(fs, inst.OpCode)

    if len(oprs) != len(values):
        raise Exception('operand: does not match values')

    for i in range(len(oprs)):
        entry.WriteOperand(data, oprs[i], values[i])

    return inst

def OpCodeHandlerPrivate(data):
    op = data.Instruction.OpCode
    entry = data.TableEntry

    handler = entry.Handler if entry.Handler != None else DefaultOpCodeHandler
    inst = handler(data)

    if inst == None:
        inst = DefaultOpCodeHandler(data)

    return inst

def OpCodeHandler(op, args):
    entry = edao_fa_op_table[op]
    fs = fafile

    data = HandlerData(HANDLER_REASON_ASSEMBLE)
    data.Instruction    = Instruction(op)
    data.Arguments      = list(args)
    data.FileStream     = fs
    data.TableEntry     = entry

    data.Instruction.OperandFormat = entry.Operand

    data.FileStream = BytesStream().openmem()

    #print(entry.OpName)
    inst = OpCodeHandlerPrivate(data)

    data.FileStream.seek(0)
    fs.write(data.FileStream.read())

    return inst
