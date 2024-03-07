import{_ as e,r as t,o as c,c as p,a as n,d as o,w as r,b as s,e as i}from"./app-O5zpLLCr.js";const l={},d=n("h1",{id:"src-print-cpp",tabindex:"-1"},[n("a",{class:"header-anchor",href:"#src-print-cpp","aria-hidden":"true"},"#"),s(" src/print.cpp")],-1),u=n("h2",{id:"namespaces",tabindex:"-1"},[n("a",{class:"header-anchor",href:"#namespaces","aria-hidden":"true"},"#"),s(" Namespaces")],-1),k=n("thead",null,[n("tr",null,[n("th",null,"Name")])],-1),v=i(`<h2 id="source-code" tabindex="-1"><a class="header-anchor" href="#source-code" aria-hidden="true">#</a> Source code</h2><div class="language-cpp line-numbers-mode" data-ext="cpp"><pre class="language-cpp"><code><span class="token macro property"><span class="token directive-hash">#</span><span class="token directive keyword">include</span> <span class="token string">&quot;zeroerr/print.h&quot;</span></span>
<span class="token macro property"><span class="token directive-hash">#</span><span class="token directive keyword">include</span> <span class="token string">&lt;iostream&gt;</span></span>

<span class="token keyword">namespace</span> zeroerr <span class="token punctuation">{</span>


Printer<span class="token operator">&amp;</span> <span class="token function">getStdoutPrinter</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token punctuation">{</span>
    <span class="token keyword">static</span> Printer <span class="token function">printer</span><span class="token punctuation">(</span>std<span class="token double-colon punctuation">::</span>cout<span class="token punctuation">)</span><span class="token punctuation">;</span>
    <span class="token keyword">return</span> printer<span class="token punctuation">;</span>
<span class="token punctuation">}</span>

Printer<span class="token operator">&amp;</span> <span class="token function">getStderrPrinter</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token punctuation">{</span>
    <span class="token keyword">static</span> Printer <span class="token function">printer</span><span class="token punctuation">(</span>std<span class="token double-colon punctuation">::</span>cerr<span class="token punctuation">)</span><span class="token punctuation">;</span>
    <span class="token keyword">return</span> printer<span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token punctuation">}</span>  <span class="token comment">// namespace zeroerr</span>
</code></pre><div class="line-numbers" aria-hidden="true"><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div><div class="line-number"></div></div></div><hr><p>Updated on 2024-03-07 at 23:54:54 +0000</p>`,4);function m(h,b){const a=t("RouterLink");return c(),p("div",null,[d,u,n("table",null,[k,n("tbody",null,[n("tr",null,[n("td",null,[n("strong",null,[o(a,{to:"/zh/Namespaces/namespacezeroerr.html"},{default:r(()=>[s("zeroerr")]),_:1})])])])])]),v])}const f=e(l,[["render",m],["__file","print_8cpp.html.vue"]]);export{f as default};
