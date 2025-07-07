import re
from bs4 import BeautifulSoup # it is a python library to parse HTML
import sys
import pandas as pd
from io import StringIO

DIFF_FOUND=False

"""
Extracts the signatures from the html file of the manually written documentation
"""
def extract_signatures(doc_file):
    with open(doc_file, 'r', encoding='utf-8') as file:
        html_content = file.read()

    soup = BeautifulSoup(html_content, 'html.parser')
    tables = soup.find_all('table')

    signatures = {}
    for table in tables:
        header = table.find('tr')
        if header and len(header.find_all('td')) == 4:
            headers = [td.text.strip() for td in header.find_all('td')]
            if headers == ['Method', 'Returns', 'Signature', 'Description'] or headers == ['Method', 'Return Type', 'Signature(s)', 'Description']:
                df = pd.read_html(StringIO(str(table)))[0]      # using pandas to make sure there are no skipped cells
                for index, row in df.iterrows():
                    method = str(row[0]) if not pd.isna(row[0]) else ""
                    returns = normalize(str(row[1])) if not pd.isna(row[1]) else ""
                    signature = normalize(str(row[2])) if not pd.isna(row[2]) else ""
                    if method not in signatures:
                        signatures[method] = {'returns': [], 'signatures': []}
                    signatures[method]['returns'].append(returns)
                    signatures[method]['signatures'].append(f"{method} ({signature})")
            return signatures
    return {}

"""
This function checks if the signatures in the doxygen file match the signatures collected from the html file
"""
def check_signatures(doxygen_file, doc_signatures):
    with open(doxygen_file, 'r', encoding='utf-8') as file:
        doxygen_content = file.read()

    soup = BeautifulSoup(doxygen_content, 'html.parser')

    correct = True
    for table in soup.find_all('table', class_='memberdecls'):
        rows = table.find_all('tr')
        for row in rows:
            cols = row.find_all('td')
            if len(cols) == 2:
                return_type = normalize(cols[0].text.strip())
                method_signature = normalize(cols[1].text.strip())
                match = re.match(r'([^)]+\))', method_signature) # making sure we don't have any extra information after the ) like const
                if match:
                    method_signature = match.group(1)
                method_name_match = re.match(r'(\w+)\s*\(', method_signature)   # extract method name
                if method_name_match:
                    method_name = method_name_match.group(1)

                    # check if signature is in html file
                    if method_name in doc_signatures.keys():
                        if return_type not in doc_signatures[method_name]['returns']:
                            print(f"- Return type mismatch for method \"{method_name}\"")
                            print(f"Doxygen return type   : {return_type}")
                            print(f"Documentation returns : {doc_signatures[method_name]['returns']} \n")
                            correct = False
                        if method_signature not in doc_signatures[method_name]['signatures']:
                            print(f"- Signature mismatch for method \"{method_name}\" ")
                            print(f"Doxygen signature        : {method_signature}")
                            print(f"Documentation signatures : {doc_signatures[method_name]['signatures']} \n")
                            correct = False
    return correct

"""
Used to normalize the text by removing extra spaces, new lines, tabs, etc.
"""
def normalize(text):
    ' '.join(text.replace("\n", "").replace("\t", "").replace("\xa0", " ").split())
    text = re.sub(r'\s*\*\s*', '* ', text)      # make sure * is followed by a space and doesn't have space before it
    text = re.sub(r'\s*&\s*', '& ', text)
    text = re.sub(r'\s*,\s*', ', ', text)
    text = re.sub(r'::', '', text)          # remove ::
    text = re.sub(r'\s+', ' ', text)
    return text

def main(argv):
    if len(argv) != 2:
        print("Usage: python3 compare_signatures.py <doc_file> <doxygen_file>")
        sys.exit(1)

    doc_file = argv[0]
    doxygen_file = argv[1]
    print("Checking function signatures for \n  Doxygen file: ", doxygen_file, "\n  Documentation file: ", doc_file, "\n")

    signatures = extract_signatures(doc_file)
    if not signatures:
        print("No matching table found in the documentation")
        sys.exit(1)

    if not check_signatures(doxygen_file, signatures):
        sys.exit(1)

if __name__ == "__main__":
    main(sys.argv[1:])
