import subprocess
import traceback
import sys
import glob
import os
from ctypes import windll
kernel32 = windll.kernel32

jt_bin = sys.argv[1] if len(sys.argv) >= 2 else 'jt'
jt_bin = '../proj/vs13/Debug/jt'

llvm_path = 'C:/Users/Alex/Assembling/LLVM-3.5/vs13/Release/bin'
llvm_as = os.path.join(llvm_path, 'llvm-as')
llvm_lli = os.path.join(llvm_path, 'lli')

class Interpreter:
	def __init__(self, source_filename=None, gen_llvm=None):
		cmd = [jt_bin, '-s']
		if source_filename:
			cmd.append(source_filename)
		if isinstance(gen_llvm, str):
			cmd.append('--gen_llvm')
			cmd.append(gen_llvm)
		self.proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

	def run(self, cmd):
		out, err = self.proc.communicate(input=bytes(cmd, 'utf-8'))

		lines = []
		for line in out.decode('utf-8').strip().split('\n'):
			lines.append(line.strip())

		return lines[-1]

class Test:
	__slots__ = ['input', 'output']

	def __init__(self, filename):
		self.input = ''
		self.output = None
		with open(filename) as f:
			for line in f:
				if line.startswith('>'):
					self.output = line[1:].strip()
					break
				if not len(line.strip()):
					continue
				self.input += line

debug_test = 'simple-var.txt'

if debug_test:
	test = Test(debug_test)
	print ('Prepared test %s' % debug_test)
	debug_file = os.path.join(os.path.dirname(jt_bin), '../jt-input.txt')
	o = open(debug_file, 'w')
	o.write(test.input)
	o.close()
	exit()

tests = glob.glob('*.txt')
for test_file in tests:
	try:
		test = Test(test_file)

		reports_dir = 'reports'
		if not os.path.exists(reports_dir):
			os.makedirs(reports_dir)

		ll_assembly = os.path.join(reports_dir, os.path.basename(test_file) + '.as')
		interp = Interpreter(gen_llvm=ll_assembly)
		out_res = interp.run(test.input)



		if test.output != out_res:
			print ('Test %s own interpreter failed' % test_file)
			print ('  Expected %s, got %s' % (test.output, out_res))
		else:
			print ('Test %s own interpreter passed' % test_file)
	except:
		print ('Test %s own interpreter failed' % test_file, traceback.format_exc())

	try:
		subprocess.check_call([llvm_as, ll_assembly])

		lli = subprocess.Popen([llvm_lli, ll_assembly + '.bc'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		out_res, err = lli.communicate()
		out_res = out_res.decode('utf-8').strip()

		if test.output != out_res:
			print ('Test %s llvm interpreter failed' % test_file)
			print ('  Expected %s, got %s' % (test.output, out_res))
		else:
			print ('Test %s llvm interpreter passed' % test_file)
	except:
		print ('Test %s llvm interpreter failed' % test_file, traceback.format_exc())
