import { defineClientConfig } from '@vuepress/client'

export default defineClientConfig({
  enhance({ app, router, siteData }) {
    router.addRoute({path: '/',redirect: '/zh/'})
  },
  setup() {},
  rootComponents: [],
})