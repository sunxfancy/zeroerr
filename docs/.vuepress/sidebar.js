import path, { resolve } from 'path'
import fs from 'fs'
import readline from 'readline'

const sidebarZh = {
    '/zh/Pages/': [
        {
            text: '指南',
            prefix: '/zh/Pages/',
            children: [
                'md_docs_zh_introduction',
                'md_docs_zh_get_started',
                'md_docs_zh_print',
                'md_docs_zh_log',
                'md_docs_zh_unittest',
            ]
        }
    ],
}

const sidebarEn = {
    '/en/Pages/': [
        {
            text: 'Guide',
            prefix: '/en/Pages/',
            children: [
                'md_docs_en_introduction',
                'md_docs_en_get_started',
                'md_docs_en_print',
                'md_docs_en_log',
                'md_docs_en_unittest',
            ]
        }
    ],
}

export async function getSidebar (locale) {
    var sb = Object.assign({}, locale === 'zh' ? sidebarZh : sidebarEn);

    var search_path = path.join(__dirname , '..');

    async function getTitle(file_path) {
        var stream = fs.createReadStream(file_path, {encoding: 'utf-8'});
        var reader = readline.createInterface({input: stream});
        var counter = 0; var line = null;
        return new Promise((resolve, reject) => {
            reader.on('line', function(l) {
                if (counter === 1) {
                    line = l;
                    reader.close();
                }
                counter++;
            });
            reader.on('close', function() {
                resolve(line.substring(7));
            });
            reader.on('error', function(err) {
                reject(err);
            });
        });
    }

    async function getFileList(search_path, name) {
        const dirpath = path.join(search_path, locale, name);
        console.log(dirpath);
        var result = [];
        var files = fs.readdirSync(dirpath);
        for (const file of files) {
            const file_path = path.join(dirpath, file);
            console.log(file_path);
            var stat = fs.statSync(file_path);
            if (stat.isFile() && file.endsWith('.md') && file !== 'index.md') {
                var title = await getTitle(file_path);
                result.push({title: title, path: file.substring(0, file.length - 3)});
            }
        }
        return result;   
    }

    async function genSidebar(name, obj, label) {
        var files = await getFileList(search_path, name);
        files.sort(function(a, b) {
            return a.title.localeCompare(b.title);
        });

        var list = [{
            text: label,
            prefix: `/${locale}/${name}/`
        }];

        var children = [`/${locale}/${name}/`];

        files.forEach(function(file) {
            children.push(file.path)
        });
        console.log(children);

        list[0].children = children;

        obj[`/${locale}/${name}`] = list;
    }
    // await genSidebar('Pages', sb, locale === 'zh' ? '指南': 'Guide');
    await genSidebar('Classes', sb, locale === 'zh' ? '参考': 'Reference');
    await genSidebar('Namespaces', sb, locale === 'zh' ? '参考': 'Reference');
    console.log(sb);

    return sb;
}