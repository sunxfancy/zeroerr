import { defineUserConfig } from 'vuepress'
import { defaultTheme } from '@vuepress/theme-default'
import { getSidebar } from './sidebar'
import { navbarZh, navbarEn } from './navbar'

export default defineUserConfig({
    // set site base to default value
  base: '/zeroerr/',
  locales: {
    '/zh/': {
        lang: 'zh-CN',
        title: 'ZeroErr 零误框架',
        description: '一款轻量级多用途C++单元测试/日志/打印框架',
    },
    '/en/': {
        lang: 'en-US',
        title: 'ZeroErr',
        description: 'A lightweight C++ unit test / logging / print framework',
    },
  },
  theme: defaultTheme({
    logo: 'logo.svg',
    repo: 'sunxfancy/zeroerr',
    docsDir: 'docs',

    // theme-level locales config
    locales: {
      /**
       * English locale config
       *
       * As the default locale of @vuepress/theme-default is English,
       * we don't need to set all of the locale fields
       */
      '/en/': {
        // navbar
        navbar: navbarEn,
        // sidebar
        sidebar: await getSidebar('en'),
        // page meta
        editLinkText: 'Edit this page on GitHub',
      },

      /**
       * Chinese locale config
       */
      '/zh/': {
        // navbar
        navbar: navbarZh,
        selectLanguageName: '简体中文',
        selectLanguageText: '选择语言',
        selectLanguageAriaLabel: '选择语言',
        // sidebar
        sidebar: await getSidebar('zh'),
        // custom containers
        tip: '提示',
        warning: '注意',
        danger: '警告',
        // 404 page
        notFound: [
          '这里什么都没有',
          '我们怎么到这来了？',
          '这是一个 404 页面',
          '看起来我们进入了错误的链接',
        ],
        backToHome: '返回首页',
        // a11y
        openInNewWindow: '在新窗口打开',
        toggleColorMode: '切换颜色模式',
        toggleSidebar: '切换侧边栏',
      },
    },
  }),

})