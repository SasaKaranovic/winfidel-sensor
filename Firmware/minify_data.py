import os
import shutil
import minify_html
from rjsmin import jsmin

class Minify:
    def __init__(self, work_dir, raw_directory, min_directory):
        self.work_dir = work_dir
        self.raw_directory = raw_directory
        self.min_directory = min_directory
        self._bytes_saved = { 'html': 0, 'js': 0, 'css': 0 }

        print(f"Working directory: {self.work_dir}")
        print(f"Source directory: {self.raw_directory}")
        print(f"Data directory: {self.min_directory}")

    def run(self):
        print("--- Running minifier ---")
        self._copy_source()

        minify_files = self._find_minify_candidates(self.min_directory)

        for file in minify_files:
            self._minify(file)
        print("---")

    def show_stats(self, key=None):
        if key is not None:
            print(f"Saved {self._bytes_saved[key]} bytes by minifying `.{key}` files")
        else:
            total = 0
            for key in self._bytes_saved:
                print(f"Saved {self._bytes_saved[key]} bytes by minifying `.{key}` files")
                total = total + self._bytes_saved[key]
            print(f"Total {total} bytes saved")

    def _find_minify_candidates(self, folder):
        minify_candidates = []
        for root, dirs, files in os.walk(folder):
            for file in files:
                if file.lower().endswith(('.htm', '.html', '.js', '.css')):
                    minify_candidates.append(os.path.join(root, file))

        return minify_candidates

    def _update_saved_count(self, category, amount):
        self._bytes_saved[category] = self._bytes_saved[category] + amount

    def _minify(self, file):
            if file.lower().endswith(('.html', '.htm')):
                content = self._read_file(file)
                content_minified = self._min_html(content)
                print(f"  -> Minified {file} [saved {len(content)-len(content_minified)} bytes]")
                self._write_file(file, content_minified)
            elif file.lower().endswith('.js'):
                content = self._read_file(file)
                content_minified = self._min_js(content)
                self._write_file(file, content_minified)
                print(f"  -> Minified {file} [saved {len(content)-len(content_minified)} bytes]")
            else:
                print(f"  -> No minifier available for file {file}")

    def _min_html(self, content):
        size_pre = len(content)
        content = minify_html.minify(content, minify_js=True, remove_processing_instructions=True)
        size_after = len(content)
        self._update_saved_count('html', size_pre-size_after)
        return content

    def _min_js(self, content):
        size_pre = len(content)
        content = jsmin(content)
        size_after = len(content)
        self._update_saved_count('js', size_pre-size_after)
        return content

    def _copy_source(self):
        shutil.copytree(self.raw_directory, self.min_directory, dirs_exist_ok=True)

    def _read_file(self, file):
        with open(file, mode="r", encoding="utf-8") as f:
            content = f.read()
        return content

    def _write_file(self, file, content):
        with open(file, mode="w", encoding="utf-8") as f:
            f.write(content)

    def _list_dir(self, filetype):
        filtered = []
        files = os.listdir(self.min_directory)
        for file in files:
            if file.endswith(f'.{filetype}'):
                filepath = os.path.join(self.min_directory, file)
                filtered.append(filepath)
        return filtered

def minify(source, target, env):
    # work_dir = os.path.dirname(os.path.abspath(__file__))
    work_dir = os.getcwd()
    raw_directory = os.path.join(work_dir, 'data_pre')
    min_directory = os.path.join(work_dir, 'data')

    print("-- Removing previous data directory")
    shutil.rmtree(min_directory, ignore_errors=True)

    print("-- Running data minify script")

    mc = Minify(work_dir, raw_directory, min_directory)
    mc.run()
    mc.show_stats()

Import("env")
minify(None, None, None)
