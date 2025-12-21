// Generate TOC for non-index pages
document.addEventListener('DOMContentLoaded', function() {
    const headers = document.querySelectorAll('main h2, main h3, main h4, main h5, main h6');
    if (headers.length === 0) return;
    
    // Create TOC container
    const tocContainer = document.createElement('div');
    tocContainer.className = 'toc-container';
    
    const tocTitle = document.createElement('h3');
    tocTitle.textContent = '目录';
    tocContainer.appendChild(tocTitle);
    
    const tocList = document.createElement('ul');
    tocList.className = 'toc';
    
    // Generate TOC items
    headers.forEach((header, index) => {
        // Generate unique ID if not present
        let id = header.id;
        if (!id) {
            const text = header.textContent.trim();
            id = text.toLowerCase()
                .replace(/\s+/g, '-')
                .replace(/[^\w\-]/g, '');
            header.id = id;
        }
        
        const tocItem = document.createElement('li');
        const level = parseInt(header.tagName.substring(1));
        tocItem.className = 'toc-level' + Math.min(level - 1, 4);
        
        const tocLink = document.createElement('a');
        tocLink.href = '#' + id;
        tocLink.textContent = header.textContent;
        
        tocItem.appendChild(tocLink);
        tocList.appendChild(tocItem);
    });
    
    tocContainer.appendChild(tocList);
    
    // Insert TOC into sidebar
    const sidebar = document.getElementById('toc-sidebar');
    if (sidebar) {
        sidebar.appendChild(tocContainer);
    }
    
    // Setup TOC scrolling and active state
    setupTOCScrolling();
});

function setupTOCScrolling() {
    const tocLinks = document.querySelectorAll('.toc a');
    const headings = document.querySelectorAll('main h2, main h3, main h4, main h5, main h6');
    
    if (tocLinks.length === 0) return;
    
    // 点击目录项时的处理
    tocLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href').substring(1);
            const targetElement = document.getElementById(targetId);
            
            if (targetElement) {
                const offset = targetElement.getBoundingClientRect().top + window.pageYOffset - 100;
                window.scrollTo({ top: offset, behavior: 'smooth' });
                
                // 更新活动状态
                tocLinks.forEach(l => l.classList.remove('active'));
                this.classList.add('active');
            }
        });
    });
    
    // 滚动时更新目录高亮
    function updateActiveTOC() {
        const scrollPosition = window.pageYOffset + 150;
        
        let activeHeading = null;
        headings.forEach(heading => {
            if (heading.offsetTop <= scrollPosition) {
                activeHeading = heading;
            }
        });
        
        if (activeHeading) {
            tocLinks.forEach(link => {
                link.classList.remove('active');
                if (link.getAttribute('href') === '#' + activeHeading.id) {
                    link.classList.add('active');
                }
            });
        }
    }
    
    // 节流函数
    function throttle(func, wait) {
        let timeout;
        return function executedFunction(...args) {
            const later = () => {
                clearTimeout(timeout);
                func(...args);
            };
            clearTimeout(timeout);
            timeout = setTimeout(later, wait);
        };
    }
    
    window.addEventListener('scroll', throttle(updateActiveTOC, 100));
    
    // 初始化时也更新一次活动状态
    setTimeout(updateActiveTOC, 100);
}