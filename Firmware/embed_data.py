import os
import io
import shutil
import gzip
from datetime import datetime
from itertools import islice

# Todo: Add option to chose between generating h+c files or .ino file
# Todo: Add option to configure custom server handlers

class espembed:
    def __init__(self, data_directory, output_directory):
        self.data_directory = data_directory
        self.output_directory = output_directory
        self.files = {}
        self.max_line_length = 60
        self.supported_extensions = {
            ".htm": { "gzip": False, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "text/html", "ctype": "uint8_t"},
            ".html":{ "gzip": False, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "text/html", "ctype": "uint8_t"},
            ".js":  { "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "text/javascript", "ctype": "uint8_t"},
            ".css": { "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "text/css", "ctype": "uint8_t"},
            ".ico": { "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_image, "encoding": "image/x-icon", "ctype": "uint8_t"},
            ".png": { "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_image, "encoding": "image/png", "ctype": "uint8_t"},
            ".jpg": { "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_image, "encoding": "image/jpeg", "ctype": "uint8_t"},
            ".jpeg":{ "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_image, "encoding": "image/jpeg", "ctype": "uint8_t"},
            ".gif": { "gzip": True, "template_processor": None, "handler_generator": self.gen_handler_image, "encoding": "image/gif", "ctype": "uint8_t"},
            ".ttf": { "gzip": False, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "font/ttf", "ctype": "uint8_t"},
            ".woff": { "gzip": False, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "font/woff", "ctype": "uint8_t"},
            ".woff2": { "gzip": False, "template_processor": None, "handler_generator": self.gen_handler_default, "encoding": "font/woff", "ctype": "uint8_t"},
        }
        self.web_handler_links = []
        self.web_data_header = ""
        self.web_data_content = ""
        # self.output_path = output_directory
        self.output_filename = "binary_data.ino"

    def add_file(self, filepath, handler, encoding=None, template_processor=None, handler_generator=None):
        extension = os.path.splitext(filepath)[1]
        if extension not in self.supported_extensions.keys():
            print(f"File `{filepath}` is not in the supported extensions list. Skipping.")
            return None

        # Option to override default encoding
        if encoding is None:
            encoding = self.supported_extensions[extension]['encoding']

        # Option to override default template_processor
        if template_processor is None:
            template_processor = self.supported_extensions[extension]['template_processor']

        # Option to override default handler_generator
        if handler_generator is None:
            if self.supported_extensions[extension]['handler_generator'] is None:
                handler_generator = self.gen_handler_default
            else:
                handler_generator = self.supported_extensions[extension]['handler_generator']


        # Read file contents
        with open(filepath, mode="rb") as f:
            content = f.read()

        # If gzip is enabled
        if self.supported_extensions[extension]['gzip']:
            print(len(content))
            content = gzip.compress(content)
            print(len(content))
            gzipped = True
        else:
            gzipped = False

        item = {
                    'binary_data' : content,
                    "gzipped": gzipped,
                    'size': len(content),
                    'handler': handler ,
                    'encoding': encoding,
                    'ctype': self.supported_extensions[extension]['ctype'],
                    'template_processor': template_processor,
                    'handler_generator': handler_generator
                }
        print(f"\t- File content length {len(content)} (gzipped: {gzipped}))")
        self.files[handler] = item

    def add_directory(self, directory, base_start):
        for root, dirs, files in os.walk(directory):
            for file in files:
                extension = os.path.splitext(file)[1]
                if extension in self.supported_extensions.keys():
                    handler_base = root.replace(base_start, '').replace('\\', '/')
                    handler = f"{handler_base}/{file}"
                    filepath = os.path.join(root, file)
                    print(f"-- Adding handler for `{handler}` located at `{filepath}`")
                    self.add_file(filepath, handler)

    def binary_data_to_hex_string(self, data):
        # Providing compatibility for pre 3.12
        def _batched(iterable, n):
            if n < 1:
                raise ValueError('n must be at least one')
            it = iter(iterable)
            while batch := tuple(islice(it, n)):
                yield batch

        ret = ','.join('0x{:02X}'.format(x) for x in data)
        ret = ",\n".join(", ".join(l) for l in _batched(ret.split(","), 16))
        return ret

    def generate_c_const_name(self, handler):
        ret = f"web_data_{handler}"
        ret = ret.replace('/', '_')
        ret = ret.replace('\\', '_')
        ret = ret.replace('-', '_')
        ret = ret.replace('.', '_')
        ret = ret.replace('__', '_')
        return ret

    def write_file_data(self, handler, data):
        dec_name = self.generate_c_const_name(handler)
        data_arr_str = self.binary_data_to_hex_string(data)

        header = f""
        content = f""
        item = { 'handler': handler, 'header':header, 'content':content }
        self.web_handler_links.append(item)

    def gen_handler_default(self, data_const_name, item):
        gzip = ""
        if item['gzipped']:
            gzip = 'response->addHeader("Content-Encoding", "gzip");\n\t\t'

        if item['template_processor'] is None:
            template_processor = ""
        else:
            template_processor = f", {item['template_processor']}"

          # request->send_P(200, "{encoding}", {variable_name});
        ret = f'''server.on("{item['handler']}", HTTP_GET, [](AsyncWebServerRequest *request){{
        AsyncWebServerResponse *response = request->beginResponse_P(200, "{item['encoding']}", {data_const_name}, {item['size']}{template_processor});
        {gzip}request->send(response);
    }});'''
        return ret

    def gen_handler_image(self, data_const_name, item):
        gzip = ""
        if item['gzipped']:
            gzip = 'response->addHeader("Content-Encoding", "gzip");\n\t\t'

        ret = f'''server.on("{item['handler']}", HTTP_GET, [](AsyncWebServerRequest *request){{
        AsyncWebServerResponse *response = request->beginResponse_P(200, "{item['encoding']}", {data_const_name}, {item['size']});
        {gzip}request->send(response);
    }});'''
        return ret

    def set_template_processor(self, extension, function_name):
        try:
            extension = extension.lower()
            self.supported_extensions[extension]['template_processor'] = function_name
        except KeyError as e:
            print(f"Extensions `{extension}` is not supported.")
            return

    def generate_output_file(self, filepath):
        forward_declarations = io.StringIO()
        data_section = io.StringIO()
        request_handlers = io.StringIO()

        data_section.write("\n\n//---- BEGIN DATA SECTION ----\n\n")
        request_handlers.write("\n\n//---- Request handler ----\n")
        request_handlers.write("void server_init_handlers(void)\n{\n")


        # Iterate over all items and generate content for output file
        for handler, item in self.files.items():
            '''
            # Write forward declarations
            forward_declarations.write('\n\n//---- Forward delcarations ----\n\n')
            for _, item in self.files.items():
                forward_declarations.write(f"// Data for handler `{item['handler']}`\n")
                forward_declarations.write(f"extern const char {item['c_data_const_name']}[{item['size']}];\n")
            '''

            # --- Write item to data section
            # todo: Optionally we could gzip/compress data here before embedding
            c_data_const_name = self.generate_c_const_name(handler)
            c_hex_str_data = self.binary_data_to_hex_string(item['binary_data'])
            c_server_handler = item['handler_generator'](c_data_const_name, self.files[handler])

            data_section.write(f"// Content of `{handler}`\n")
            data_section.write(f"const {item['ctype']} {c_data_const_name}[{item['size']}] PROGMEM = {{\n{c_hex_str_data}\n}};\n")
            # f.write(f"const uint8_t {item['c_data_const_name']}[] PROGMEM = {{\n{item['c_hex_str_data']}\n}};\n")

            # --- Create request handler for item
            # Write server request handlers
            request_handlers.write(f"\t{c_server_handler}\n")

        # -- Finalize data and handler sections
        data_section.write("\n\n//---- END DATA SECTION ----\n\n")
        request_handlers.write("}\n")

        # Merge content
        content = data_section.getvalue() + '\n\n' + request_handlers.getvalue()
        self.write_ino_file(content);
        data_section.close()
        request_handlers.close()
        forward_declarations.close()

    def write_ino_file(self, content):
        filename = os.path.join(self.output_directory, self.output_filename)
        with open(filename, mode='w', encoding='utf-8') as f:
            f.write("// This file was auto generated using ESP32 Data Embed script\n")
            f.write("// Please do not manually modify this file\n")
            f.write("// since any changes to this file will be overwritten on next build.\n")
            f.write(f"// Generated on {datetime.today().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            f.write(content)
            f.write("\n// ---- End of auto generated file\n")

    def run(self):
        for handler, file_data in self.files.items():
            self.write_file_data(handler, file_data['binary_data'])

        self.generate_output_file(self.output_directory)

def main(data_dir, src_dir):
    embed = espembed(data_dir, src_dir)
    embed.set_template_processor('.htm', 'template_const_processor')
    embed.set_template_processor('.html', 'template_const_processor')
    embed.add_directory('data', 'data')
    embed.run()

Import("env")
print("\n--- Embedding binary data...")
path = os.getcwd()
data_dir = os.path.join(path, 'data')
src_dir = os.path.join(path, 'src')
main(data_dir, src_dir)
print("--- Embedding binary data: complete")
