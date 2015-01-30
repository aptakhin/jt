import subprocess
import traceback
import sys
import glob

jt_bin = sys.argv[1] if len(sys.argv) >= 2 else 'jt'
jt_bin = '../proj/vs13/Debug/jt.exe'

class Interpreter:
	def __init__(self, source_filename=None):
		cmd = [jt_bin, '-s']
		if source_filename:
			cmd.append(source_filename)
		self.proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

	def write(self, cmd):
		out, err = self.proc.communicate(input=bytes(cmd, 'utf-8'))

		lines = []
		for line in out.decode('utf-8').strip().split('\n'):
			lines.append(line.strip())

		return lines[-1]

	def test(self, test_file):
		try:
			code = ''
			eq = None
			with open(test_file) as f:
				for line in f:
					if line.startswith('>'):
						eq = line[1:].strip()
						break
					if not len(line.strip()):
						continue
					code += line

			#print (code)
			#print ('@', eq)
			res = self.write(code)

			if res != eq:
				print ('Test %s failed' % test_file)
			else:
				print ('Test %s passed' % test_file)

		except:
			print (traceback.format_exc())

tests = glob.glob('*.txt')
for test_file in tests:
	interp = Interpreter()
	interp.test(test_file)
