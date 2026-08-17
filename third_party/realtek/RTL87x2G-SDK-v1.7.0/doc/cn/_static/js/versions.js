/* ============= 状态管理 ============= */
const docConfig = {
    isMultiTarget: false,
    fetchedData: {},
};
const eleSelectors = {
    starget: null,
    sversion: null,
};


/* ============= URL 解析 ============= */

/**
 * parse current URL and extract target & version
 * @returns {{ currentTarget: string|null, currentVersion: string|null }}
 */
function parseUrlParts() {
    let currentTarget = null;
    let currentVersion = null;
    const urlSegments = window.location.pathname.toLowerCase().split('/');

    if (docConfig.isMultiTarget) {
        currentTarget = Object.keys(docConfig.fetchedData).find(t =>
            urlSegments.includes(t.toLowerCase())
        ) || null;

        if (currentTarget) {
            const currentTargetObj = docConfig.fetchedData[currentTarget];
            const currentVersionList = (currentTargetObj && currentTargetObj.version) ? currentTargetObj.version : [];
            currentVersion = currentVersionList.find(v =>
                urlSegments.includes(v.toLowerCase())
            ) || null;
        }
    } else {
        currentVersion = docConfig.fetchedData.find(v =>
            urlSegments.includes(v.toLowerCase())
        ) || null;
    }

    return { currentTarget, currentVersion };
}


/* ============= 选择器操作 ============= */

/**
 * generate <select> inner html
 * @param {HTMLSelectElement} selector
 * @param {Array<string | {optionValue: string, optionLabel: string, optionUrl?: string}>} dataSource
 * @param {string} selectedValue
 */
function populateSelector(selector, dataSource, selectedValue) {
    const optionsHTML = dataSource.map(item => {
        const optionValue = typeof item === 'string' ? item : item.optionValue;
        const optionLabel = typeof item === 'string' ? item : item.optionLabel;
        const dataUrl = item.optionUrl ? `data-url="${item.optionUrl}"` : '';
        return `<option value="${optionValue}" ${dataUrl} ${optionValue === selectedValue ? 'selected' : ''}>${optionLabel}</option>`;
    }).join('');
    selector.innerHTML = optionsHTML;
}

/**
 * update version selector by current target
 */
function updateVersionSelector(target, selectedVersion) {
    let versionList = [];
    if (docConfig.isMultiTarget) {
        const targetObj = docConfig.fetchedData[target];
        versionList = (targetObj && targetObj.version) ? targetObj.version : [];
    } else {
        versionList = docConfig.fetchedData;
    }

    const firstVersion = versionList.length > 0 ? versionList[0] : "latest";
    const versionValue = selectedVersion || firstVersion;
    populateSelector(eleSelectors.sversion, versionList, versionValue);
}

/**
 * init both selectors
 */
function initSelectors(targetValue, versionValue) {
    if (docConfig.isMultiTarget) {
        const targetKeys = Object.keys(docConfig.fetchedData);
        populateSelector(eleSelectors.starget, targetKeys, targetValue);
        updateVersionSelector(targetValue, versionValue);
    } else {
        updateVersionSelector(null, versionValue);
    }
}


/* ============= 切换事件 ============= */

function onTargetChange() {
    const { currentTarget, currentVersion } = parseUrlParts();
    const selectedTarget = eleSelectors.starget.value;
    const targetObj = docConfig.fetchedData[selectedTarget];
    const versionsList = (targetObj && targetObj.version) ? targetObj.version : [];
    const defaultVersion = versionsList.length > 0 ? versionsList[0] : "latest";

    let newUrl = window.location.href;
    newUrl = newUrl.replace(/\/(cn|en)\//i, '/en/');
    if (currentTarget) {
        newUrl = newUrl.replace(new RegExp(currentTarget, 'i'), selectedTarget);
    }
    if (currentVersion) {
        newUrl = newUrl.replace(new RegExp(currentVersion, 'i'), defaultVersion);
    }
    newUrl = newUrl.replace(new RegExp(`/${defaultVersion}/.*`, 'i'), `/${defaultVersion}/index.html`);
    window.location.href = newUrl;
}

function onVersionChange() {
    const { currentTarget, currentVersion } = parseUrlParts();
    const selectedVersion = eleSelectors.sversion.value;

    let newUrl = window.location.href;
    if (currentVersion) {
        newUrl = newUrl.replace(new RegExp(currentVersion, 'i'), selectedVersion);
    }
    window.location.href = newUrl;
}


/* ============= 数据获取 ============= */

async function fetchVersionData() {
    let fetchUrl = "https://docs.realmcu.com/sdk/version.txt";
    try {
        const response = await fetch(fetchUrl);
        const text = await response.text();

        if (docConfig.isMultiTarget) {
            docConfig.fetchedData = JSON.parse(text);
        } else {
            docConfig.fetchedData = text.split(/[\r\n]+/).map(item => item.trim()).filter(Boolean);
        }

        const { currentTarget, currentVersion } = parseUrlParts();
        initSelectors(currentTarget, currentVersion);
    } catch (error) {
        console.error('Fail to fetch data:', error);
    }
}


/* ============= Init version selector ============= */
document.addEventListener('DOMContentLoaded', (event) => {
    docConfig.isMultiTarget = window.isMultitarget == "True";
    eleSelectors.starget = document.getElementById("target-selector");
    eleSelectors.sversion = document.getElementById("version-selector");
    fetchVersionData();
})
