import re
from bs4 import BeautifulSoup # it is a python library to parse HTML
import sys

"""
- search for:
---IN CODERINFO
<tr>
<td>
<p align="center"><b>Method</b></p></td>
<td>
<p align="center"><b>Returns</b></p></td>
<td>
<p align="center"><b>Signature</b></p></td>
<td>
<p align="center"><b>Description</b></p></td></tr>
<tr>

OR : 

<tr>
    <td>
        <center><b>Method</b></center>
    </td>
    <td>
        <center><b>Returns</b></center>
    </td>
    <td>
        <center><b>Signature</b></center>
    </td>
    <td>
        <center><b>Description</b></center>
    </td>
</tr>
                
- then for each row:
- check the signature with the doxugen output (!! there could be many signatures for the same function)
"""

def clean_string(s):
    return ' '.join(s.replace("\n", "").replace("\t", "").replace("\xa0", " ").split())

def extract_signatures(html_file):
    with open(html_file, 'r', encoding='utf-8') as file:
        html_content = file.read()

    soup = BeautifulSoup(html_content, 'html.parser')
    tables = soup.find_all('table')

    for table in tables:
        rows = table.find_all('tr')
        if rows and len(rows[0].find_all('td')) == 4:
            signatures = {}
            current_method = None
            for row in rows:
                cols = row.find_all('td')
                if len(cols) == 4:
                    method = clean_string(cols[0].text.strip())
                    returns = clean_string(cols[1].text.strip())
                    signature = clean_string(cols[2].text.strip())
                    if method not in signatures:
                        signatures[method] = {
                            'returns': [],
                            'signatures': [],
                        }
                    signatures[method]['returns'].append(returns)
                    signatures[method]['signatures'].append(method + " (" + signature + ")")
                    current_method = method
                elif len(cols) == 3 and current_method:
                    returns = clean_string(cols[0].text.strip())
                    signature = clean_string(cols[1].text.strip())
                    signatures[current_method]['returns'].append(returns)
                    signatures[current_method]['signatures'].append(current_method + " (" + signature + ")")
                elif len(cols) == 2 and current_method:
                    signature = clean_string(cols[0].text.strip())
                    signatures[current_method]['signatures'].append(current_method + " (" + signature + ")")
            return signatures
    print("No matching table found")
    return {}

def extract_doxygen_signatures(doxygen_file):
    with open(doxygen_file, 'r', encoding='utf-8') as file:
        doxygen_content = file.read()

    soup = BeautifulSoup(doxygen_content, 'html.parser')
    signatures = {}

    for table in soup.find_all('table', class_='memberdecls'):
        rows = table.find_all('tr')
        for row in rows:
            cols = row.find_all('td')
            if len(cols) == 2:
                return_type = clean_string(cols[0].text.strip())
                method_signature = clean_string(cols[1].text.strip())
                method_name_match = re.match(r'(\w+)\s*\(', method_signature)
                if method_name_match:
                    method_name = method_name_match.group(1)
                    if method_name not in signatures:
                        signatures[method_name] = {
                            'returns': set(),
                            'signatures': set()
                        }
                    signatures[method_name]['returns'].add(return_type)
                    signatures[method_name]['signatures'].add(method_signature)

    for method in signatures:
        signatures[method]['returns'] = list(signatures[method]['returns'])
        signatures[method]['signatures'] = list(signatures[method]['signatures'])

    return signatures

def normalize_signature(signature):
    signature = re.sub(r'\s*\*\s*', '*', signature)
    signature = re.sub(r'\s*,\s*', ', ', signature)
    return signature

def compare_signatures(html_signatures, doxygen_signatures):
    # TODO rather check in the doxygen for the html but directly => avoid too much loops
    for method, html_details in html_signatures.items():
        if method in doxygen_signatures:
            doxygen_details = doxygen_signatures[method]

            html_sigs = set(normalize_signature(sig) for sig in html_details['signatures'])
            doxygen_sigs = set(normalize_signature(sig) for sig in doxygen_details['signatures'])
            html_returns = set(html_details['returns'])
            doxygen_returns = set(doxygen_details['returns'])

            if html_sigs != doxygen_sigs:
                print(f"Mismatch in signatures for method {method}:")
                print(f"HTML: {html_sigs}")
                print(f"Doxygen: {doxygen_sigs}")

            if html_returns != doxygen_returns:
                print(f"Mismatch in return types for method {method}:")
                print(f"HTML: {html_returns}")
                print(f"Doxygen: {doxygen_returns}")
        else:
            print(f"Method {method} not found in Doxygen signatures")

    for method in doxygen_signatures:
        if method not in html_signatures:
            print(f"Method {method} found in Doxygen signatures but not in HTML signatures")


def main(argv):
    if len(argv) != 2:
        print("Usage: python3 compare_signatures.py <html_file> <doxygen_file>")
        sys.exit(1)

    html_file = argv[0]
    doxygen_file = argv[1]
    print("Checking signatures for html file: ", html_file, "and doxygen file: ", doxygen_file)

    signatures = extract_signatures(html_file)
    """for signature in signatures:
        print(signature)
        print(signatures[signature])

    print('---------------------')"""

    signatures2 = extract_doxygen_signatures(doxygen_file)
    """for signature in signatures2:
        print(signature)
        print(signatures2[signature])

    print('---------------------')"""
    compare_signatures(signatures, signatures2)


if __name__ == "__main__":
    main(sys.argv[1:])