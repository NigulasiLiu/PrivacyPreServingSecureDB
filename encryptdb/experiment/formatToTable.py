#! /usr/bin/python3
import re
import os
import os.path

def format_to_table():
    result_filename = './result.txt'
    output_dir = './output'
    output_files = [os.path.join(output_dir, item) for item in os.listdir(output_dir)]
    output_files = [item for item in output_files if os.path.isfile(item) and item.endswith('txt')]

    titles = ['tag', 'round', 'n']
    rows = []

    for filename in output_files:
        basename = re.split('\.', os.path.basename(filename))[0]
        words = re.split('-', basename)
        record = {'n': words[0], 'round': words[1], 'tag': words[2]}
        with open(filename, 'r') as f:
            line = f.readline()
            while line:
                words = [i for i in re.split(' |\n', line) if i != '']
                if len(words) == 3 and re.match('\[.+\]', words[0]) == None:
                    tit = f'{words[0]}({words[2]})'
                    if tit not in titles:
                        titles.append(tit)
                    record[tit] = words[1]
                line = f.readline()
        rows.append(record)

    with open(result_filename, 'w') as f:
        f.write('\t'.join(titles) + '\n')
        for r in rows:
            line = '\t'.join([r[item] for item in titles if item in r]) + '\n'
            f.write(line)

    print('Done')

if __name__ == '__main__':
    format_to_table()