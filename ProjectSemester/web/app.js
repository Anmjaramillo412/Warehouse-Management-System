// ============================================================
// MODULE NAVIGATION
// ============================================================

function openModule(module) {

    const dashboard =
        document.querySelector(".dashboard");

    const moduleView =
        document.getElementById("module-view");

    const content =
        document.getElementById("module-content");

    dashboard.style.display = "none";

    moduleView.classList.remove("hidden");

    content.innerHTML = "";


    // ========================================================
    // MATERIAL MANAGER
    // ========================================================

    if (module === "materials") {

        content.innerHTML = `

            <h1>
                Material Manager
            </h1>

            <p>
                Manage materials stored in the system.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateMaterial()">
                    Create Material
                </button>

                <button onclick="showModifyMaterial()">
                    Modify Material
                </button>

                <button onclick="showDeleteMaterial()">
                    Delete Material
                </button>

                <button onclick="showSearchMaterial()">
                    Search Material
                </button>

                <button onclick="displayMaterials()">
                    Display Materials
                </button>

            </div>

            <div id="material-content">
            </div>
        `;
    }

    // ========================================================
    // PRODUCT MANAGER
    // ========================================================

    else if (module === "products") {

        content.innerHTML = `

            <h1>
                Product Manager
            </h1>

            <p>
                Manage products and their Bills of Materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateProduct()">
                    Create Product
                </button>

                <button onclick="showDisplayProducts()">
                    Display Products
                </button>

                <button onclick="showDeleteProduct()">
                    Delete Product
                </button>

                <button onclick="showSellProduct()">
                    Sell Product
                </button>

            </div>

            <div id="product-content">
            </div>

        `;
    }

    // ========================================================
    // WAREHOUSE MANAGER
    // ========================================================

    else if (module === "warehouses") {

        content.innerHTML = `

            <h1>
                Warehouse Manager
            </h1>

            <p>
                Manage warehouses and storage locations.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateWarehouse()">
                    Create Warehouse
                </button>

                <button onclick="showDeleteWarehouse()">
                    Delete Warehouse
                </button>

                <button onclick="displayWarehouses()">
                    Display Warehouses
                </button>

            </div>

            <div id="warehouse-content">
            </div>
        `;
    }


    // ========================================================
    // INVENTORY MANAGER
    // ========================================================

    else if (module === "inventory") {

        content.innerHTML = `

            <h1>
                Inventory Manager
            </h1>

            <p>
                Manage material movements and inventory.
            </p>

            <div class="module-buttons">

                <button onclick="showGoodsReceipt()">
                    Goods Receipt
                </button>

                <button onclick="showGoodsIssue()">
                    Goods Issue
                </button>

                <button onclick="showTransferMaterial()">
                    Transfer Material
                </button>

                <button onclick="displayInventory()">
                    Display Inventory
                </button>

                <button onclick="inventoryCheck()">
                    Inventory Check
                </button>

            </div>

            <div id="inventory-content">
            </div>
        `;
    }


    // ========================================================
    // SUPPLIER MANAGER
    // ========================================================

    else if (module === "suppliers") {

        content.innerHTML = `

            <h1>
                Supplier Manager
            </h1>

            <p>
                Manage suppliers referenced by materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateSupplier()">
                    Create Supplier
                </button>

                <button onclick="showModifySupplier()">
                    Modify Supplier
                </button>

                <button onclick="displaySuppliers()">
                    Display Suppliers
                </button>

                <button onclick="showDeleteSupplier()">
                    Delete Supplier
                </button>

            </div>

            <div id="supplier-content">
            </div>
        `;
    }

    // ========================================================
    // PROCUREMENT
    // ========================================================

    else if (module === "procurement") {

        content.innerHTML = `

            <h1>
                Procurement
            </h1>

            <p>
                Project production needs and manage purchase orders.
            </p>

            <div class="module-buttons">

                <button onclick="showNewProjection()">
                    New Projection
                </button>

                <button onclick="showProjections()">
                    Open Projections
                </button>

                <button class="button-muted" onclick="showArchivedProjections()">
                    Archived Projections
                </button>

                <button class="button-accent" onclick="showNewProcurementOrder()">
                    New Order
                </button>

                <button class="button-accent" onclick="showProcurementOrders()">
                    Open Orders
                </button>

                <button class="button-accent" onclick="showConfirmedOrders()">
                    Confirmed Orders
                </button>

                <button class="button-muted" onclick="showArchivedOrders()">
                    Archived Orders
                </button>

            </div>

            <div id="procurement-content">
            </div>
        `;
    }

    // ========================================================
    // DATA MANAGEMENT
    // ========================================================

    else if (module === "data") {

        content.innerHTML = `

            <h1>
                Data Management
            </h1>

            <p>
                Save and recover system data.
            </p>


            <div class="module-buttons">

                <button onclick="saveData()">
                    Save Data
                </button>

                <button onclick="loadData()">
                    Load Data
                </button>

                <div class="data-logging-option">

                    <label>

                        <input
                            type="checkbox"
                            id="log-data-operations"
                            onchange="setDataLogging()"
                        >

                        Log Save/Load operations

                    </label>

                </div>

            </div>


            <div id="data-logging-message">
            </div>

        `;


        loadDataLoggingState();
    }
}

// ============================================================
// CREATE SUPPLIER FORM
// ============================================================

function showCreateSupplier() {

    const content =
        document.getElementById("supplier-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Supplier
            </h2>

            <label>
                Name
            </label>

            <input
                type="text"
                id="supplier-name"
                placeholder="Enter Supplier Name"
            >


            <label>
                Address
            </label>

            <input
                type="text"
                id="supplier-address"
                placeholder="Enter Address"
            >


            <label>
                Country
            </label>

            <input
                type="text"
                id="supplier-country"
                placeholder="Enter Country"
            >


            <label>
                Contact Name
            </label>

            <input
                type="text"
                id="supplier-contact-name"
                placeholder="Enter Contact Name"
            >


            <label>
                Contact Email
            </label>

            <input
                type="email"
                id="supplier-contact-email"
                placeholder="Enter Contact Email"
            >


            <label>
                Website
            </label>

            <input
                type="text"
                id="supplier-website"
                placeholder="https://..."
            >


            <label>
                Ordering Method
            </label>

            <div class="checkbox-group">

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="Online">
                    Online
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="Email">
                    Email
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="PO">
                    PO
                </label>

            </div>


            <label>
                Payment Method
            </label>

            <select id="supplier-payment-method">
                <option value="Invoice">Invoice</option>
                <option value="Pre-payment">Pre-payment</option>
                <option value="Online">Online</option>
            </select>


            <label>
                Lead Time (weeks)
            </label>

            <input
                type="number"
                id="supplier-lead-time"
                min="0"
                value="0"
            >


            <div class="form-actions">

                <button onclick="createSupplier()">
                    Create Supplier
                </button>

            </div>


            <div id="supplier-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE SUPPLIER
// ============================================================

async function createSupplier() {

    const name =
        document.getElementById(
            "supplier-name"
        ).value.trim();

    const address =
        document.getElementById(
            "supplier-address"
        ).value.trim();

    const country =
        document.getElementById(
            "supplier-country"
        ).value.trim();

    const contactName =
        document.getElementById(
            "supplier-contact-name"
        ).value.trim();

    const contactEmail =
        document.getElementById(
            "supplier-contact-email"
        ).value.trim();

    const website =
        document.getElementById(
            "supplier-website"
        ).value.trim();

    const orderingMethods =
        Array.from(
            document.querySelectorAll(
                ".supplier-ordering-method:checked"
            )
        ).map(checkbox => checkbox.value);

    const paymentMethod =
        document.getElementById(
            "supplier-payment-method"
        ).value;

    const leadTimeWeeks =
        Number(
            document.getElementById(
                "supplier-lead-time"
            ).value
        ) || 0;

    const message =
        document.getElementById(
            "supplier-message"
        );


    if (!name) {

        message.textContent =
            "Supplier name is required.";

        return;
    }


    const supplier = {

        name: name,

        address: address,

        country: country,

        contactName: contactName,

        contactEmail: contactEmail,

        website: website,

        orderingMethods: orderingMethods,

        paymentMethod: paymentMethod,

        leadTimeWeeks: leadTimeWeeks
    };


    try {

        const response =
            await fetch(
                "/api/suppliers/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(supplier)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Supplier created successfully.";

            showCreateSupplier();
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// MODIFY SUPPLIER FORM
// ============================================================

function showModifySupplier() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Supplier
            </h2>

            <label>
                Supplier
            </label>

            <select id="ms-lookup-name">
                <option value="">Loading suppliers...</option>
            </select>

            <div class="form-actions">

                <button onclick="loadSupplierForModify()">
                    Load Supplier
                </button>

            </div>

            <div id="modify-supplier-form">
            </div>

        </div>
    `;

    populateSupplierSelect("ms-lookup-name");
}

// ============================================================
// LOAD SUPPLIER FOR MODIFY
// ============================================================

async function loadSupplierForModify() {

    const name =
        document.getElementById(
            "ms-lookup-name"
        ).value;

    const modifyForm =
        document.getElementById(
            "modify-supplier-form"
        );

    if (!name) {

        modifyForm.innerHTML = `
            <p>
                Please select a supplier.
            </p>
        `;

        return;
    }

    modifyForm.innerHTML = `
        <p>
            Loading supplier...
        </p>
    `;

    try {

        const response =
            await fetch(
                `/api/suppliers/${encodeURIComponent(name)}`
            );

        const data =
            await response.json();

        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }

        const orderingMethods =
            data.orderingMethods || [];

        modifyForm.innerHTML = `

            <input type="hidden" id="ms-name" value="${escapeHtml(data.name)}">

            <p>
                Supplier:
                <strong>${escapeHtml(data.name)}</strong>
            </p>

            <label>
                Address
            </label>

            <input type="text" id="ms-address"
                value="${escapeHtml(data.address || "")}">

            <label>
                Country
            </label>

            <input type="text" id="ms-country"
                value="${escapeHtml(data.country || "")}">

            <label>
                Contact Name
            </label>

            <input type="text" id="ms-contact-name"
                value="${escapeHtml(data.contactName || "")}">

            <label>
                Contact Email
            </label>

            <input type="email" id="ms-contact-email"
                value="${escapeHtml(data.contactEmail || "")}">

            <label>
                Website
            </label>

            <input type="text" id="ms-website"
                value="${escapeHtml(data.website || "")}">

            <label>
                Ordering Method
            </label>

            <div class="checkbox-group">

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="Online"
                        ${orderingMethods.includes("Online") ? "checked" : ""}>
                    Online
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="Email"
                        ${orderingMethods.includes("Email") ? "checked" : ""}>
                    Email
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="PO"
                        ${orderingMethods.includes("PO") ? "checked" : ""}>
                    PO
                </label>

            </div>

            <label>
                Payment Method
            </label>

            <select id="ms-payment-method">
                <option value="Invoice" ${data.paymentMethod === "Invoice" ? "selected" : ""}>Invoice</option>
                <option value="Pre-payment" ${data.paymentMethod === "Pre-payment" ? "selected" : ""}>Pre-payment</option>
                <option value="Online" ${data.paymentMethod === "Online" ? "selected" : ""}>Online</option>
            </select>

            <label>
                Lead Time (weeks)
            </label>

            <input type="number" id="ms-lead-time" min="0"
                value="${data.leadTimeWeeks || 0}">

            <div class="form-actions">

                <button onclick="modifySupplierSubmit()">
                    Save Changes
                </button>

            </div>

            <div id="modify-supplier-message">
            </div>
        `;
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// MODIFY SUPPLIER (SUBMIT)
// ============================================================

async function modifySupplierSubmit() {

    const name =
        document.getElementById(
            "ms-name"
        ).value;

    const address =
        document.getElementById(
            "ms-address"
        ).value.trim();

    const country =
        document.getElementById(
            "ms-country"
        ).value.trim();

    const contactName =
        document.getElementById(
            "ms-contact-name"
        ).value.trim();

    const contactEmail =
        document.getElementById(
            "ms-contact-email"
        ).value.trim();

    const website =
        document.getElementById(
            "ms-website"
        ).value.trim();

    const orderingMethods =
        Array.from(
            document.querySelectorAll(
                ".ms-ordering-method:checked"
            )
        ).map(checkbox => checkbox.value);

    const paymentMethod =
        document.getElementById(
            "ms-payment-method"
        ).value;

    const leadTimeWeeks =
        Number(
            document.getElementById(
                "ms-lead-time"
            ).value
        ) || 0;

    const message =
        document.getElementById(
            "modify-supplier-message"
        );

    const supplier = {

        name: name,

        address: address,

        country: country,

        contactName: contactName,

        contactEmail: contactEmail,

        website: website,

        orderingMethods: orderingMethods,

        paymentMethod: paymentMethod,

        leadTimeWeeks: leadTimeWeeks
    };

    try {

        const response =
            await fetch(
                "/api/suppliers/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(supplier)
                }
            );

        const responseText =
            await response.text();

        if (response.ok) {

            message.textContent =
                "Supplier successfully modified.";
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY SUPPLIERS
// ============================================================

async function displaySuppliers() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `
        <p>Loading suppliers...</p>
    `;


    try {

        const response =
            await fetch("/api/suppliers");


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.suppliers ||
            data.suppliers.length === 0
        ) {

            content.innerHTML = `

                <div class="empty-message">

                    No suppliers available.

                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Suppliers
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Name</th>
                            <th>Country</th>
                            <th>Contact</th>
                            <th>Email</th>
                            <th>Website</th>
                            <th>Ordering</th>
                            <th>Payment</th>
                            <th>Lead Time</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (const supplier of data.suppliers) {

            html += `

                <tr>

                    <td>${escapeHtml(supplier.name)}</td>
                    <td>${escapeHtml(supplier.country || "")}</td>
                    <td>${escapeHtml(supplier.contactName || "")}</td>
                    <td>${escapeHtml(supplier.contactEmail || "")}</td>
                    <td>${escapeHtml(supplier.website || "")}</td>
                    <td>${escapeHtml((supplier.orderingMethods || []).join(", "))}</td>
                    <td>${escapeHtml(supplier.paymentMethod || "")}</td>
                    <td>${supplier.leadTimeWeeks} week(s)</td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE SUPPLIER FORM
// ============================================================

function showDeleteSupplier() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Supplier
            </h2>

            <label>
                Supplier
            </label>

            <select id="delete-supplier-name">
                <option value="">Loading suppliers...</option>
            </select>

            <div class="form-actions">

                <button onclick="deleteSupplier()">
                    Delete Supplier
                </button>

            </div>

            <div id="delete-supplier-message">
            </div>

        </div>
    `;

    populateSupplierSelect("delete-supplier-name");
}

// ============================================================
// DELETE SUPPLIER
// ============================================================

async function deleteSupplier() {

    const name =
        document.getElementById(
            "delete-supplier-name"
        ).value;

    const message =
        document.getElementById(
            "delete-supplier-message"
        );


    if (!name) {

        message.textContent =
            "Please select a supplier.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/suppliers/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({ name: name })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Supplier deleted successfully.";

            showDeleteSupplier();
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA LOGGING STATE
// ============================================================

async function loadDataLoggingState()
{
    try
    {
        const response =
            await fetch(
                "/api/data/logging"
            );


        const data =
            await response.json();


        if (!response.ok)
        {
            return;
        }


        const checkbox =
            document.getElementById(
                "log-data-operations"
            );


        if (checkbox)
        {
            checkbox.checked =
                data.enabled;
        }
    }
    catch (error)
    {
        console.error(
            "Could not load logging state:",
            error
        );
    }
}

// ============================================================
// SAVE DATA
// ============================================================

async function saveData() {

    const message =
            document.getElementById(
                "data-logging-message"
            );

    try {

        const response =
            await fetch(
                "/api/data/save",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "✓ Data saved successfully.";

        }
        else {

            message.textContent =
                "Error saving data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA
// ============================================================

async function loadData() {
    
    const message =
        document.getElementById(
            "data-logging-message"
        );

    try {

        const response =
            await fetch(
                "/api/data/load",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {
            message.textContent =
                "✓ Data loaded successfully.";

        }
        else {

            message.textContent =
                "Error loading data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE MATERIAL FORM
// ============================================================

function showCreateMaterial() {

    const content =
        document.getElementById("material-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="material-id"
                placeholder="###-###### or ######-00"
                maxlength="10"
            >

            <small>
                Format: ###-###### or ######-00
            </small>


            <label>
                Material Name
            </label>

            <input
                type="text"
                id="material-name"
                placeholder="Enter Material Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="material-description"
                placeholder="Enter Material Description"
                rows="4"></textarea>


            <label>
                Unit of Measure
            </label>

            <input
                type="text"
                id="material-uom"
                value="pcs"
                placeholder="pcs"
            >


            <label>
                Category
            </label>

            <input
                type="text"
                id="material-category"
                placeholder="Enter Category"
            >


            <label>
                Material Type
            </label>

            <select
                id="material-type"
                onchange="toggleDrawingNumberField('material')">

                <option value="Standard Part">Standard Part</option>
                <option value="Design Part">Design Part</option>
                <option value="PCB">PCB</option>

            </select>


            <div id="material-drawing-number-group" class="hidden">

                <label>
                    Drawing Number
                </label>

                <input
                    type="text"
                    id="material-drawing-number"
                    placeholder="e.g. 123-ASM-4567"
                >

                <small>
                    Format: ###-ASM-####, ###-PAR-####,
                    I-BU#-##, OWI-BU#-##, or ###-PCB-####
                </small>

            </div>


            <label>
                Manufacturer
            </label>

            <input
                type="text"
                id="material-manufacturer"
                placeholder="Enter Manufacturer"
            >


            <label>
                Manufacturer Part Number
            </label>

            <input
                type="text"
                id="material-manufacturer-pn"
                placeholder="Enter Manufacturer Part Number"
            >


            <label>
                Supplier
            </label>

            <div class="combobox">

                <input
                    type="text"
                    id="material-supplier-search"
                    autocomplete="off"
                    placeholder="Search supplier by name..."
                    oninput="handleSupplierSearchInput('material')"
                    onfocus="renderSupplierOptions('material')"
                    onblur="hideSupplierOptionsDelayed('material')"
                >

                <input type="hidden" id="material-supplier">

                <div id="material-supplier-options"
                    class="combobox-options hidden">
                </div>

            </div>

            <small>
                No supplier listed?
                <a href="#" onclick="openModule('suppliers'); return false;">
                    Create one first.
                </a>
            </small>


            <label>
                Supplier Part Number
            </label>

            <input
                type="text"
                id="material-supplier-pn"
                placeholder="Enter Supplier Part Number"
            >


            <label>
                Material Photo
            </label>

            <input
                type="file"
                id="material-photo"
                accept="image/*"
            >

            <label class="checkbox-label">

                <input
                    type="checkbox"
                    id="material-active"
                    checked
                >

                Active Material

            </label>


            <div class="form-actions">

                <button
                    onclick="createMaterial()">

                    Create Material

                </button>

            </div>


            <div id="material-message">
            </div>

        </div>
    `;

    initSupplierCombobox("material");
}

// ============================================================
// TOGGLE DRAWING NUMBER FIELD
// ============================================================
// Drawing Number only applies to Design Part / PCB materials.

function toggleDrawingNumberField(prefix) {

    const type =
        document.getElementById(
            prefix + "-type"
        ).value;

    const group =
        document.getElementById(
            prefix + "-drawing-number-group"
        );

    const requiresDrawing =
        (type === "Design Part" || type === "PCB");

    group.classList.toggle(
        "hidden",
        !requiresDrawing);
}

// ============================================================
// POPULATE SUPPLIER SELECT
// ============================================================

async function populateSupplierSelect(selectId, selectedName) {

    const select =
        document.getElementById(selectId);

    if (!select) {
        return;
    }

    try {

        const response =
            await fetch("/api/suppliers");

        const data =
            await response.json();

        const suppliers =
            data.suppliers || [];

        if (suppliers.length === 0) {

            select.innerHTML = `
                <option value="">
                    No suppliers available
                </option>
            `;

            return;
        }

        select.innerHTML =
            `<option value="">Select a supplier</option>` +
            suppliers.map(supplier => `
                <option value="${escapeHtml(supplier.name)}">
                    ${escapeHtml(supplier.name)}
                </option>
            `).join("");

        if (selectedName) {
            select.value = selectedName;
        }
    }
    catch (error) {

        console.error(error);

        select.innerHTML = `
            <option value="">
                Could not load suppliers
            </option>
        `;
    }
}

// ============================================================
// SUPPLIER SEARCH COMBOBOX
// ============================================================
// Used by the Material create/modify forms to search for an
// existing supplier by name instead of picking from a long list.
// The hidden "<prefix>-supplier" input only ever holds a name
// that exactly matches an existing supplier - it is cleared
// whenever the visible search text does not match one.

let supplierCache = null;

async function loadSupplierCache(forceRefresh) {

    if (supplierCache && !forceRefresh) {
        return supplierCache;
    }

    try {

        const response =
            await fetch("/api/suppliers");

        const data =
            await response.json();

        supplierCache =
            data.suppliers || [];
    }
    catch (error) {

        console.error(error);

        supplierCache = [];
    }

    return supplierCache;
}

async function initSupplierCombobox(prefix, selectedName) {

    await loadSupplierCache(true);

    const hiddenInput =
        document.getElementById(prefix + "-supplier");

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    hiddenInput.value = selectedName || "";
    searchInput.value = selectedName || "";
}

function renderSupplierOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    const optionsBox =
        document.getElementById(prefix + "-supplier-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const filterText =
        searchInput.value.trim().toLowerCase();

    const suppliers =
        supplierCache || [];

    const matches =
        suppliers.filter(supplier =>
            supplier.name.toLowerCase().includes(filterText)
        );

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No suppliers found
            </div>
        `;
    }
    else {

        optionsBox.innerHTML =
            matches.map(supplier => `
                <div class="combobox-option"
                    onmousedown="selectSupplierOption('${prefix}', '${escapeHtml(supplier.name)}')">
                    ${escapeHtml(supplier.name)}
                </div>
            `).join("");
    }

    optionsBox.classList.remove("hidden");
}

function handleSupplierSearchInput(prefix) {

    renderSupplierOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    const hiddenInput =
        document.getElementById(prefix + "-supplier");

    const suppliers =
        supplierCache || [];

    const typedName =
        searchInput.value.trim();

    const exactMatch =
        suppliers.find(supplier =>
            supplier.name === typedName
        );

    hiddenInput.value =
        exactMatch ? exactMatch.name : "";
}

function selectSupplierOption(prefix, name) {

    document.getElementById(
        prefix + "-supplier"
    ).value = name;

    document.getElementById(
        prefix + "-supplier-search"
    ).value = name;

    document.getElementById(
        prefix + "-supplier-options"
    ).classList.add("hidden");
}

function hideSupplierOptionsDelayed(prefix) {

    // Delay so a click (onmousedown) on an option
    // still registers before the dropdown disappears.

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-supplier-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

// ============================================================
// MATERIAL SEARCH COMBOBOX
// ============================================================
// Generic Material ID lookup, used anywhere a Material ID needs
// to be entered (Search/Delete/Modify Material, Transfer Material,
// Goods Receipt/Issue line items), so materials can be found by
// typing part of the ID or name instead of remembering the exact
// ID. The hidden "<prefix>-material-id" input only ever holds an
// ID that exactly matches an existing material - it is cleared
// whenever the visible search text does not match one exactly.

let materialCache = null;

async function loadMaterialCache(forceRefresh) {

    if (materialCache && !forceRefresh) {
        return materialCache;
    }

    try {

        const response =
            await fetch("/api/materials");

        const data =
            await response.json();

        materialCache =
            data.materials || [];
    }
    catch (error) {

        console.error(error);

        materialCache = [];
    }

    return materialCache;
}

async function initMaterialCombobox(prefix, selectedID) {

    await loadMaterialCache();

    const hiddenInput =
        document.getElementById(prefix + "-material-id");

    const searchInput =
        document.getElementById(prefix + "-material-id-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    hiddenInput.value = selectedID || "";
    searchInput.value = selectedID || "";
}

function renderMaterialOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-material-id-search");

    const optionsBox =
        document.getElementById(prefix + "-material-id-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const filterText =
        searchInput.value.trim().toLowerCase();

    const materials =
        materialCache || [];

    const matches =
        materials.filter(material =>
            material.id.toLowerCase().includes(filterText) ||
            (material.name || "").toLowerCase().includes(filterText)
        );

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No materials found
            </div>
        `;
    }
    else {

        const limited =
            matches.slice(0, 30);

        optionsBox.innerHTML =
            limited.map(material => `
                <div class="combobox-option"
                    onmousedown="selectMaterialOption('${prefix}', '${escapeHtml(material.id)}')">
                    <strong>${escapeHtml(material.id)}</strong>
                    &nbsp;&mdash;&nbsp;${escapeHtml(material.name)}
                </div>
            `).join("") +
            (matches.length > limited.length
                ? `<div class="combobox-option combobox-empty">
                        ${matches.length - limited.length} more - keep typing to narrow down
                   </div>`
                : "");
    }

    optionsBox.classList.remove("hidden");
}

function handleMaterialSearchInput(prefix) {

    renderMaterialOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-material-id-search");

    const hiddenInput =
        document.getElementById(prefix + "-material-id");

    const materials =
        materialCache || [];

    const typedText =
        searchInput.value.trim();

    const exactMatch =
        materials.find(material =>
            material.id === typedText
        );

    hiddenInput.value =
        exactMatch ? exactMatch.id : "";
}

function selectMaterialOption(prefix, id) {

    document.getElementById(
        prefix + "-material-id"
    ).value = id;

    document.getElementById(
        prefix + "-material-id-search"
    ).value = id;

    document.getElementById(
        prefix + "-material-id-options"
    ).classList.add("hidden");
}

function hideMaterialOptionsDelayed(prefix) {

    // Delay so a click (onmousedown) on an option
    // still registers before the dropdown disappears.

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-material-id-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

// ============================================================
// MATERIAL ID COMBOBOX MARKUP HELPER
// ============================================================
// Returns the HTML for a Material ID search box with the given
// prefix, so every form that needs one builds it the same way.

function materialComboboxHtml(prefix, placeholder) {

    return `
        <div class="combobox">

            <input
                type="text"
                id="${prefix}-material-id-search"
                autocomplete="off"
                placeholder="${placeholder || "Search by ID or name..."}"
                oninput="handleMaterialSearchInput('${prefix}')"
                onfocus="renderMaterialOptions('${prefix}')"
                onblur="hideMaterialOptionsDelayed('${prefix}')"
            >

            <input type="hidden" id="${prefix}-material-id">

            <div id="${prefix}-material-id-options"
                class="combobox-options hidden">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE MATERIAL
// ============================================================

async function createMaterial() {

    const id =
        document.getElementById(
            "material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "material-name"
        ).value.trim();

    const description =
        document.getElementById(
            "material-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "material-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "material-category"
        ).value.trim();

    const type =
        document.getElementById(
            "material-type"
        ).value;

    const drawingNumber =
        document.getElementById(
            "material-drawing-number"
        ).value.trim();

    const manufacturer =
        document.getElementById(
            "material-manufacturer"
        ).value.trim();

    const manufacturerPartNumber =
        document.getElementById(
            "material-manufacturer-pn"
        ).value.trim();

    const supplier =
        document.getElementById(
            "material-supplier"
        ).value.trim();

    const supplierPartNumber =
        document.getElementById(
            "material-supplier-pn"
        ).value.trim();

    const active =
        document.getElementById(
            "material-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "material-photo"
        );

    const photoFile =
        photoInput.files[0];

    let photoData = "";
    let photoName = "";

    if (photoFile) {

        photoName =
            photoFile.name;

        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    // ========================================================
    // VALIDATION
    // ========================================================

    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const drawingNumberPattern =
        /^([0-9]{3}-ASM-[0-9]{4}|[0-9]{3}-PAR-[0-9]{4}|I-BU[0-9]-[0-9]{2}|OWI-BU[0-9]-[0-9]{2}|[0-9]{3}-PCB-[0-9]{4})$/;

    const message =
        document.getElementById(
            "material-message"
        );


    if (!idPattern.test(id)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!name ||
        !category ||
        !supplier) {

        message.textContent =
            "Please fill in all required fields, including the Supplier.";

        return;
    }


    if ((type === "Design Part" || type === "PCB") &&
        !drawingNumberPattern.test(drawingNumber)) {

        message.textContent =
            "Invalid or missing Drawing Number for this material type.";

        return;
    }


    // ========================================================
    // PREPARE JSON
    // ========================================================

    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        type: type,

        drawingNumber: drawingNumber,

        manufacturer: manufacturer,

        manufacturerPartNumber: manufacturerPartNumber,

        supplier: supplier,

        supplierPartNumber: supplierPartNumber,

        photo: photoName,

        photoData: photoData,

        active: active
    };


    // ========================================================
    // SEND TO C++
    // ========================================================

    try {

        const response =
            await fetch(
                "/api/materials/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material created successfully.";


            document.getElementById(
                "material-id"
            ).value = "";

            document.getElementById(
                "material-name"
            ).value = "";

            document.getElementById(
                "material-description"
            ).value = "";

            document.getElementById(
                "material-uom"
            ).value = "pcs";

            document.getElementById(
                "material-category"
            ).value = "";

            document.getElementById(
                "material-type"
            ).value = "Standard Part";

            document.getElementById(
                "material-drawing-number"
            ).value = "";

            toggleDrawingNumberField("material");

            document.getElementById(
                "material-manufacturer"
            ).value = "";

            document.getElementById(
                "material-manufacturer-pn"
            ).value = "";

            document.getElementById(
                "material-supplier"
            ).value = "";

            document.getElementById(
                "material-supplier-search"
            ).value = "";

            document.getElementById(
                "material-supplier-pn"
            ).value = "";

            document.getElementById(
                "material-photo"
            ).value = "";

            document.getElementById(
                "material-active"
            ).checked = true;

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY MATERIALS
// ============================================================

async function displayMaterials() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `
        <p>Loading materials...</p>
    `;


    try {

        const response =
            await fetch(
                "/api/materials"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.materials ||
            data.materials.length === 0
        ) {

            content.innerHTML = `

                <div class="empty-message">

                    No materials available.

                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Materials
                </h2>

                <table class="material-table material-table-clickable">

                    <thead>

                        <tr>

                            <th>Photo</th>
                            <th>Material ID</th>
                            <th>Name</th>
                            <th>UoM</th>
                            <th>Drawing Number</th>
                            <th>Manufacturer</th>
                            <th>Mfr Part #</th>
                            <th>Supplier</th>
                            <th>Supplier Part #</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const material of data.materials
        ) {

            html += `

                <tr
                    onclick="showMaterialDetail('${escapeHtml(material.id)}')"
                    title="Click to view full material details">

                    <td>

                        ${
                            material.photo
                            ? `<img
                                    src="/${material.photo}"
                                    class="material-thumbnail"
                                    alt="Material photo"
                               >`
                            : "No photo"
                        }

                    </td>

                    <td>
                        ${escapeHtml(material.id)}
                    </td>

                    <td>
                        ${escapeHtml(material.name)}
                    </td>

                    <td>
                        ${escapeHtml(material.uom || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.drawingNumber || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.manufacturer || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.manufacturerPartNumber || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.supplier || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.supplierPartNumber || "")}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

        const tableContainer =
            content.querySelector(".material-table-container");

        if (tableContainer) {
            tableContainer.scrollLeft = 0;
        }

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// MATERIAL DETAIL DASHBOARD
// ============================================================

async function showMaterialDetail(id) {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `
        <p>Loading material...</p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${escapeHtml(data.message || "Material not found.")}
                </p>

                <div class="module-buttons">
                    <button onclick="displayMaterials()">
                        ← Back to Materials
                    </button>
                </div>
            `;

            return;
        }


        content.innerHTML = `

            <div class="material-detail">

                <div class="module-buttons material-detail-back">
                    <button onclick="displayMaterials()">
                        ← Back to Materials
                    </button>
                </div>

                <div class="material-detail-header">

                    <div class="material-detail-photo">
                        ${
                            data.photo
                            ? `<img
                                    src="/${data.photo}"
                                    alt="Material photo"
                               >`
                            : `<div class="material-detail-photo-placeholder">
                                    No photo
                               </div>`
                        }
                    </div>

                    <div class="material-detail-title">

                        <h2>
                            ${escapeHtml(data.name)}
                        </h2>

                        <p class="material-detail-id">
                            ${escapeHtml(data.id)}
                        </p>

                        <span class="status-badge ${data.active ? "status-active" : "status-inactive"}">
                            ${data.active ? "Active" : "Inactive"}
                        </span>

                    </div>

                </div>

                <div class="material-detail-grid">

                    <div class="detail-field">
                        <div class="detail-label">Description</div>
                        <div class="detail-value">${escapeHtml(data.description || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Unit of Measure</div>
                        <div class="detail-value">${escapeHtml(data.uom || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Category</div>
                        <div class="detail-value">${escapeHtml(data.category || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Type</div>
                        <div class="detail-value">${escapeHtml(data.type || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Drawing Number</div>
                        <div class="detail-value">${escapeHtml(data.drawingNumber || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Manufacturer</div>
                        <div class="detail-value">${escapeHtml(data.manufacturer || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Manufacturer Part Number</div>
                        <div class="detail-value">${escapeHtml(data.manufacturerPartNumber || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Supplier</div>
                        <div class="detail-value">${escapeHtml(data.supplier || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Supplier Part Number</div>
                        <div class="detail-value">${escapeHtml(data.supplierPartNumber || "—")}</div>
                    </div>

                </div>

            </div>
        `;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// SEARCH MATERIAL FORM
// ============================================================

function showSearchMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Search Material
            </h2>

            <label>
                Material ID
            </label>

            ${materialComboboxHtml("search")}

            <div class="form-actions">

                <button
                    onclick="searchMaterial()">

                    Search Material

                </button>

            </div>

            <div id="search-material-result">
            </div>

        </div>
    `;

    initMaterialCombobox("search");
}


// ============================================================
// SEARCH MATERIAL
// ============================================================

async function searchMaterial() {

    const id =
        document.getElementById(
            "search-material-id"
        ).value.trim();


    const result =
        document.getElementById(
            "search-material-result"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        result.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    result.innerHTML = `
        <p>
            Searching...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            result.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        result.innerHTML = `

            <div class="search-result">

                <h3>
                    Material Found
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${data.name}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${data.description}
                </p>

                <p>
                    <strong>Unit of Measure:</strong>
                    ${data.uom}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${data.category}
                </p>

                <p>
                    <strong>Type:</strong>
                    ${data.type}
                </p>

                ${
                    data.drawingNumber
                    ? `<p>
                        <strong>Drawing Number:</strong>
                        ${data.drawingNumber}
                    </p>`
                    : ""
                }

                <p>
                    <strong>Manufacturer:</strong>
                    ${data.manufacturer || ""}
                </p>

                <p>
                    <strong>Manufacturer Part Number:</strong>
                    ${data.manufacturerPartNumber || ""}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${data.supplier}
                </p>

                <p>
                    <strong>Supplier Part Number:</strong>
                    ${data.supplierPartNumber || ""}
                </p>

                <p>
                    <strong>Active:</strong>
                    ${data.active ? "Yes" : "No"}
                </p>

            </div>
        `;

    }
    catch (error) {

        console.error(error);

        result.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}


// ============================================================
// MODIFY MATERIAL FORM
// ============================================================

function showModifyMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Material
            </h2>

            <label>
                Material ID
            </label>

            ${materialComboboxHtml("modify")}

            <div class="form-actions">

                <button
                    onclick="loadMaterialForModify()">

                    Load Material

                </button>

            </div>

            <div id="modify-form">
            </div>

        </div>
    `;

    initMaterialCombobox("modify");
}

// ============================================================
// DELETE MATERIAL FORM
// ============================================================

function showDeleteMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Material
            </h2>


            <label>
                Material ID
            </label>

            ${materialComboboxHtml("delete")}


            <div class="form-actions">

                <button
                    onclick="loadMaterialForDelete()">

                    Find Material

                </button>

            </div>


            <div id="delete-content">
            </div>

        </div>
    `;

    initMaterialCombobox("delete");
}

// ============================================================
// LOAD MATERIAL FOR DELETE
// ============================================================

async function loadMaterialForDelete() {

    const id =
        document.getElementById(
            "delete-material-id"
        ).value.trim();


    const content =
        document.getElementById(
            "delete-content"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        content.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    content.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        content.innerHTML = `

            <div class="search-result">

                <h3>
                    Material to Delete
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${escapeHtml(data.name)}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${escapeHtml(data.description || "")}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${escapeHtml(data.category)}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${escapeHtml(data.supplier)}
                </p>


                <div class="delete-confirmation">

                    <p>
                        <strong>
                            Are you sure you want to delete
                            this material?
                        </strong>
                    </p>


                    <button
                        class="delete-button"
                        onclick="deleteMaterial(
                            '${data.id}'
                        )">

                        Yes, Delete Material

                    </button>


                    <button
                        class="cancel-button"
                        onclick="showDeleteMaterial()">

                        Cancel

                    </button>

                </div>

            </div>

        `;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE MATERIAL
// ============================================================

async function deleteMaterial(id) {

    const content =
        document.getElementById(
            "material-content"
        );


    try {

        const response =
            await fetch(
                "/api/materials/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        Material Deleted
                    </h3>

                    <p>
                        Material
                        <strong>${escapeHtml(id)}</strong>
                        was deleted successfully.
                    </p>

                </div>
            `;
        }
        else {

            content.innerHTML = `

                <div class="search-result">

                    <p>
                        Error: ${escapeHtml(responseText)}
                    </p>

                </div>
            `;
        }
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// LOAD MATERIAL FOR MODIFY
// ============================================================

async function loadMaterialForModify() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();


    const modifyForm =
        document.getElementById(
            "modify-form"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        modifyForm.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    modifyForm.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        modifyForm.innerHTML = `

            <div class="form-container">

                <h2>
                    Modify Material
                </h2>


                <p>
                    Material ID:
                    <strong>${data.id}</strong>
                </p>


                <label>
                    Material Name
                </label>

                <input
                    type="text"
                    id="modify-name"
                    value="${escapeHtml(data.name)}"
                >


                <label>
                    Description
                </label>

                <textarea
                    id="modify-description"
                    rows="4"
                >${escapeHtml(data.description || "")}</textarea>


                <label>
                    Unit of Measure
                </label>

                <input
                    type="text"
                    id="modify-uom"
                    value="${escapeHtml(data.uom)}"
                >


                <label>
                    Category
                </label>

                <input
                    type="text"
                    id="modify-category"
                    value="${escapeHtml(data.category)}"
                >


                <label>
                    Material Type
                </label>

                <select
                    id="modify-type"
                    onchange="toggleDrawingNumberField('modify')">

                    <option value="Standard Part"
                        ${data.type === "Standard Part" ? "selected" : ""}>
                        Standard Part
                    </option>

                    <option value="Design Part"
                        ${data.type === "Design Part" ? "selected" : ""}>
                        Design Part
                    </option>

                    <option value="PCB"
                        ${data.type === "PCB" ? "selected" : ""}>
                        PCB
                    </option>

                </select>


                <div id="modify-drawing-number-group"
                    class="${
                        (data.type === "Design Part" || data.type === "PCB")
                        ? ""
                        : "hidden"
                    }">

                    <label>
                        Drawing Number
                    </label>

                    <input
                        type="text"
                        id="modify-drawing-number"
                        value="${escapeHtml(data.drawingNumber || "")}"
                        placeholder="e.g. 123-ASM-4567"
                    >

                    <small>
                        Format: ###-ASM-####, ###-PAR-####,
                        I-BU#-##, OWI-BU#-##, or ###-PCB-####
                    </small>

                </div>


                <label>
                    Manufacturer
                </label>

                <input
                    type="text"
                    id="modify-manufacturer"
                    value="${escapeHtml(data.manufacturer || "")}"
                >


                <label>
                    Manufacturer Part Number
                </label>

                <input
                    type="text"
                    id="modify-manufacturer-pn"
                    value="${escapeHtml(data.manufacturerPartNumber || "")}"
                >


                <label>
                    Supplier
                </label>

                <div class="combobox">

                    <input
                        type="text"
                        id="modify-supplier-search"
                        autocomplete="off"
                        placeholder="Search supplier by name..."
                        oninput="handleSupplierSearchInput('modify')"
                        onfocus="renderSupplierOptions('modify')"
                        onblur="hideSupplierOptionsDelayed('modify')"
                    >

                    <input type="hidden" id="modify-supplier">

                    <div id="modify-supplier-options"
                        class="combobox-options hidden">
                    </div>

                </div>


                <label>
                    Supplier Part Number
                </label>

                <input
                    type="text"
                    id="modify-supplier-pn"
                    value="${escapeHtml(data.supplierPartNumber || "")}"
                >

                <label>
                    Material Photo
                </label>

                <input
                    type="file"
                    id="modify-photo-input"
                    accept="image/*"
                >

                <input
                    type="hidden"
                    id="modify-photo"
                    value="${escapeHtml(data.photo || "")}"
                >

                <label class="checkbox-label">

                    <input
                        type="checkbox"
                        id="modify-active"
                        ${data.active ? "checked" : ""}
                    >

                    Active Material

                </label>


                <div class="form-actions">

                    <button
                        onclick="modifyMaterial()">

                        Save Changes

                    </button>

                </div>


                <div id="modify-message">
                </div>

            </div>

        `;

        initSupplierCombobox(
            "modify",
            data.supplier);
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// HTML ESCAPE
// ============================================================

function escapeHtml(value) {

    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

// ============================================================
// CONFIRM DIALOG (in-page, instead of the browser's own
// window.confirm popup - built once and reused for every call)
// ============================================================

function showConfirmDialog(message) {

    return new Promise((resolve) => {

        let overlay =
            document.getElementById(
                "app-confirm-overlay"
            );

        if (!overlay) {

            overlay =
                document.createElement("div");

            overlay.id =
                "app-confirm-overlay";

            overlay.className =
                "app-modal-overlay hidden";

            overlay.innerHTML = `
                <div class="app-modal-box">

                    <p id="app-confirm-message">
                    </p>

                    <div class="app-modal-actions">

                        <button
                            type="button"
                            id="app-confirm-cancel"
                            class="app-modal-button-secondary">

                            Cancel

                        </button>

                        <button
                            type="button"
                            id="app-confirm-ok"
                            class="app-modal-button-primary">

                            OK

                        </button>

                    </div>

                </div>
            `;

            document.body.appendChild(overlay);
        }

        document.getElementById(
            "app-confirm-message"
        ).textContent = message;

        overlay.classList.remove("hidden");

        const okButton =
            document.getElementById("app-confirm-ok");

        const cancelButton =
            document.getElementById("app-confirm-cancel");

        function finish(result) {

            overlay.classList.add("hidden");

            okButton.onclick = null;
            cancelButton.onclick = null;

            resolve(result);
        }

        okButton.onclick = () => finish(true);
        cancelButton.onclick = () => finish(false);
    });
}

// ============================================================
// MODIFY MATERIAL
// ============================================================

async function modifyMaterial() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "modify-name"
        ).value.trim();

    const description =
        document.getElementById(
            "modify-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "modify-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "modify-category"
        ).value.trim();

    const type =
        document.getElementById(
            "modify-type"
        ).value;

    const drawingNumber =
        document.getElementById(
            "modify-drawing-number"
        ).value.trim();

    const manufacturer =
        document.getElementById(
            "modify-manufacturer"
        ).value.trim();

    const manufacturerPartNumber =
        document.getElementById(
            "modify-manufacturer-pn"
        ).value.trim();

    const supplier =
        document.getElementById(
            "modify-supplier"
        ).value.trim();

    const supplierPartNumber =
        document.getElementById(
            "modify-supplier-pn"
        ).value.trim();

    const active =
        document.getElementById(
            "modify-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "modify-photo-input"
        );

    const photoFile =
        photoInput.files[0];

    let photoPath =
        document.getElementById(
            "modify-photo"
        ).value;


    let photoData = "";


    if (photoFile)
    {
        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    const message =
        document.getElementById(
            "modify-message"
        );

    const drawingNumberPattern =
        /^([0-9]{3}-ASM-[0-9]{4}|[0-9]{3}-PAR-[0-9]{4}|I-BU[0-9]-[0-9]{2}|OWI-BU[0-9]-[0-9]{2}|[0-9]{3}-PCB-[0-9]{4})$/;

    if (!name ||
        !category ||
        !supplier) {

        message.textContent =
            "Please fill in all required fields, including the Supplier.";

        return;
    }


    if ((type === "Design Part" || type === "PCB") &&
        !drawingNumberPattern.test(drawingNumber)) {

        message.textContent =
            "Invalid or missing Drawing Number for this material type.";

        return;
    }


    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        type: type,

        drawingNumber: drawingNumber,

        manufacturer: manufacturer,

        manufacturerPartNumber: manufacturerPartNumber,

        supplier: supplier,

        supplierPartNumber: supplierPartNumber,

        photo: photoPath,

        photoData: photoData,

        active: active
    };


    try {

        const response =
            await fetch(
                "/api/materials/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material successfully modified.";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE WAREHOUSE FORM
// ============================================================

function showCreateWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Warehouse Name
            </label>

            <input
                type="text"
                id="warehouse-name"
                placeholder="Enter Warehouse Name"
            >


            <div class="form-actions">

                <button
                    onclick="createWarehouse()">

                    Create Warehouse

                </button>

            </div>


            <div id="warehouse-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE WAREHOUSE
// ============================================================

async function createWarehouse() {

    const id =
        document.getElementById(
            "warehouse-id"
        ).value;

    const name =
        document.getElementById(
            "warehouse-name"
        ).value.trim();


    const message =
        document.getElementById(
            "warehouse-message"
        );


    if (!id || !name) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    const warehouse = {

        id: Number(id),

        name: name

    };


    try {

        const response =
            await fetch(
                "/api/warehouses/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(warehouse)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse created successfully.";


            document.getElementById(
                "warehouse-id"
            ).value = "";

            document.getElementById(
                "warehouse-name"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY WAREHOUSES
// ============================================================

async function displayWarehouses() {

    const content =
        document.getElementById(
            "warehouse-content"
        );


    content.innerHTML = `
        <p>
            Loading warehouses...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/warehouses"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No warehouses available.
                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Warehouses
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Warehouse ID</th>
                            <th>Warehouse Name</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <tr>

                    <td>
                        ${warehouse.id}
                    </td>

                    <td>
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE WAREHOUSE FORM
// ============================================================

function showDeleteWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="delete-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <div class="form-actions">

                <button
                    onclick="deleteWarehouse()">

                    Delete Warehouse

                </button>

            </div>


            <div id="warehouse-delete-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE WAREHOUSE
// ============================================================

async function deleteWarehouse() {

    const id =
        document.getElementById(
            "delete-warehouse-id"
        ).value;


    const message =
        document.getElementById(
            "warehouse-delete-message"
        );


    if (!id) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/warehouses/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: Number(id)
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse deleted successfully.";


            document.getElementById(
                "delete-warehouse-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// GOODS RECEIPT FORM
// ============================================================

function showGoodsReceipt() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Goods Receipt
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="receipt-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Materials
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addGoodsLine('receipt')">

                        + Add Material

                    </button>

                </div>

                <div id="receipt-lines">
                </div>

            </div>


            <label>
                Comment
            </label>

            <textarea
                id="receipt-comment"
                rows="4"
                placeholder="General comment for this receipt (applies to all materials above)"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsReceipt()">

                    Receive Goods

                </button>

            </div>


            <div id="receipt-message">
            </div>

        </div>
    `;

    initGoodsLines("receipt");
}

// ============================================================
// GOODS RECEIPT
// ============================================================

async function goodsReceipt() {

    const warehouseID =
        document.getElementById(
            "receipt-warehouse-id"
        ).value;

    const comment =
        document.getElementById(
            "receipt-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "receipt-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    const lines =
        collectGoodsLines("receipt", message);

    if (!lines) {
        return;
    }


    // --------------------------------------------------------
    // Send each line to C++ (same endpoint, one call per
    // material, sharing the same warehouse and comment)
    // --------------------------------------------------------

    message.textContent =
        "Processing...";

    const results =
        await submitGoodsLines(
            "/api/inventory/receipt",
            warehouseID,
            comment,
            lines
        );

    reportGoodsLinesResult(
        message,
        results,
        "Goods receipt"
    );

    if (results.every(result => result.ok)) {

        document.getElementById(
            "receipt-warehouse-id"
        ).value = "";

        document.getElementById(
            "receipt-comment"
        ).value = "";

        initGoodsLines("receipt");
    }
}

// ============================================================
// GOODS ISSUE FORM
// ============================================================

function showGoodsIssue() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Goods Issue
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="issue-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Materials
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addGoodsLine('issue')">

                        + Add Material

                    </button>

                </div>

                <div id="issue-lines">
                </div>

            </div>


            <label>
                Comment
            </label>

            <textarea
                id="issue-comment"
                rows="4"
                placeholder="General comment for this issue (applies to all materials above)"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsIssue()">

                    Issue Goods

                </button>

            </div>


            <div id="issue-message">
            </div>

        </div>
    `;

    initGoodsLines("issue");
}

// ============================================================
// GOODS ISSUE
// ============================================================

async function goodsIssue() {

    const warehouseID =
        document.getElementById(
            "issue-warehouse-id"
        ).value;

    const comment =
        document.getElementById(
            "issue-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "issue-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    const lines =
        collectGoodsLines("issue", message);

    if (!lines) {
        return;
    }


    // --------------------------------------------------------
    // Send each line to C++ (same endpoint, one call per
    // material, sharing the same warehouse and comment)
    // --------------------------------------------------------

    message.textContent =
        "Processing...";

    const results =
        await submitGoodsLines(
            "/api/inventory/issue",
            warehouseID,
            comment,
            lines
        );

    reportGoodsLinesResult(
        message,
        results,
        "Goods issue"
    );

    if (results.every(result => result.ok)) {

        document.getElementById(
            "issue-warehouse-id"
        ).value = "";

        document.getElementById(
            "issue-comment"
        ).value = "";

        initGoodsLines("issue");
    }
}

// ============================================================
// GOODS RECEIPT / ISSUE - MULTIPLE MATERIAL LINES
// ============================================================
// Both forms let the user add several materials at once under
// one Warehouse ID and one general Comment. Each line keeps its
// own Material ID (via the shared combobox) and Quantity. The
// backend still only knows how to receive/issue one material at
// a time, so each line is sent as its own request to the same
// endpoint, reusing the shared warehouse and comment.

let goodsLineCounters = {
    receipt: 0,
    issue: 0
};

function goodsLineRowHtml(type, rowIndex) {

    const prefix =
        `${type}-line-${rowIndex}`;

    return `
        <div class="line-item" id="${prefix}-row">

            <div class="line-item-field">

                <label>
                    Material ID
                </label>

                ${materialComboboxHtml(prefix)}

            </div>

            <div class="line-item-field line-item-field-qty">

                <label>
                    Quantity
                </label>

                <input
                    type="number"
                    id="${prefix}-quantity"
                    min="1"
                    placeholder="Qty"
                >

            </div>

            <button
                type="button"
                class="remove-line-button"
                title="Remove this material"
                onclick="removeGoodsLine('${type}', ${rowIndex})">

                &times;

            </button>

        </div>
    `;
}

function initGoodsLines(type) {

    goodsLineCounters[type] = 0;

    const container =
        document.getElementById(`${type}-lines`);

    if (!container) {
        return;
    }

    container.innerHTML = "";

    addGoodsLine(type);
}

function addGoodsLine(type) {

    const rowIndex =
        goodsLineCounters[type]++;

    const container =
        document.getElementById(`${type}-lines`);

    if (!container) {
        return;
    }

    container.insertAdjacentHTML(
        "beforeend",
        goodsLineRowHtml(type, rowIndex)
    );

    initMaterialCombobox(`${type}-line-${rowIndex}`);
}

function removeGoodsLine(type, rowIndex) {

    const container =
        document.getElementById(`${type}-lines`);

    const row =
        document.getElementById(`${type}-line-${rowIndex}-row`);

    if (!container || !row) {
        return;
    }

    // Always keep at least one line on the form.
    if (container.children.length <= 1) {
        return;
    }

    row.remove();
}

function collectGoodsLines(type, message) {

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const rows =
        Array.from(
            document.querySelectorAll(`#${type}-lines .line-item`)
        );

    if (rows.length === 0) {

        message.textContent =
            "Please add at least one material.";

        return null;
    }

    const lines = [];
    const seenIDs = new Set();

    for (const row of rows) {

        const materialID =
            row.querySelector(
                "input[id$='-material-id']"
            ).value.trim();

        const quantity =
            row.querySelector(
                "input[id$='-quantity']"
            ).value;

        if (!materialIDPattern.test(materialID)) {

            message.textContent =
                "Please select a valid Material ID (from the list) for every line.";

            return null;
        }

        if (!quantity ||
            Number(quantity) <= 0) {

            message.textContent =
                `Quantity must be greater than zero for material ${materialID}.`;

            return null;
        }

        if (seenIDs.has(materialID)) {

            message.textContent =
                `Material ${materialID} appears more than once. Combine it into a single line.`;

            return null;
        }

        seenIDs.add(materialID);

        lines.push({
            materialID: materialID,
            quantity: Number(quantity)
        });
    }

    return lines;
}

async function submitGoodsLines(endpoint, warehouseID, comment, lines) {

    const results = [];

    for (const line of lines) {

        const payload = {

            warehouseID:
                Number(warehouseID),

            materialID:
                line.materialID,

            quantity:
                line.quantity,

            comment:
                comment
        };

        try {

            const response =
                await fetch(
                    endpoint,
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body:
                            JSON.stringify(payload)
                    }
                );

            const responseText =
                await response.text();

            results.push({
                materialID: line.materialID,
                ok: response.ok,
                detail: responseText
            });
        }
        catch (error) {

            console.error(error);

            results.push({
                materialID: line.materialID,
                ok: false,
                detail: "Could not connect to the server."
            });
        }
    }

    return results;
}

function reportGoodsLinesResult(message, results, label) {

    const succeeded =
        results.filter(result => result.ok);

    const failed =
        results.filter(result => !result.ok);

    if (failed.length === 0) {

        message.textContent =
            `${label} completed successfully for ${succeeded.length} material(s).`;

        return;
    }

    message.innerHTML =
        `${succeeded.length} of ${results.length} material(s) processed successfully.<br>` +
        "Errors:<br>" +
        failed.map(result =>
            `${escapeHtml(result.materialID)}: ${escapeHtml(result.detail)}`
        ).join("<br>");
}

// ============================================================
// TRANSFER MATERIAL FORM
// ============================================================

function showTransferMaterial() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Transfer Material
            </h2>


            <label>
                Source Warehouse ID
            </label>

            <input
                type="number"
                id="transfer-source"
                placeholder="Enter Source Warehouse ID"
            >


            <label>
                Destination Warehouse ID
            </label>

            <input
                type="number"
                id="transfer-destination"
                placeholder="Enter Destination Warehouse ID"
            >


            <label>
                Material ID
            </label>

            ${materialComboboxHtml("transfer")}


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="transfer-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="transfer-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>

            <div class="form-actions">

                <button
                    onclick="transferMaterial()">

                    Transfer Material

                </button>

            </div>


            <div id="transfer-message">
            </div>

        </div>
    `;

    initMaterialCombobox("transfer");
}

// ============================================================
// TRANSFER MATERIAL
// ============================================================

async function transferMaterial() {

    const sourceWarehouseID =
        document.getElementById(
            "transfer-source"
        ).value;

    const destinationWarehouseID =
        document.getElementById(
            "transfer-destination"
        ).value;

    const materialID =
        document.getElementById(
            "transfer-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "transfer-quantity"
        ).value;

    const comment =
        document.getElementById(
            "transfer-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "transfer-message"
        );

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!sourceWarehouseID ||
        !destinationWarehouseID) {

        message.textContent =
            "Please enter both warehouse IDs.";

        return;
    }


    if (
        Number(sourceWarehouseID) ===
        Number(destinationWarehouseID)
    ) {

        message.textContent =
            "Source and destination warehouses must be different.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const transfer = {

        sourceWarehouseID:
            Number(sourceWarehouseID),

        destinationWarehouseID:
            Number(destinationWarehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment
    };

    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/transfer",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(transfer)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material transferred successfully.";

            clearInputFields([
                "transfer-source",
                "transfer-destination",
                "transfer-material-id",
                "transfer-material-id-search",
                "transfer-quantity",
                "transfer-comment"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY INVENTORY
// ============================================================

async function displayInventory() {

    const content =
        document.getElementById(
            "inventory-content"
        );


    content.innerHTML = `
        <p>
            Loading inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">

                    No inventory available.

                </div>
            `;

            return;
        }


        let html = "";


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <div class="material-table-container">

                    <h2>
                        Warehouse
                        ${warehouse.id}
                        -
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </h2>
            `;


            if (
                !warehouse.inventory ||
                warehouse.inventory.length === 0
            ) {

                html += `

                    <div class="empty-message">

                        Warehouse is empty.

                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Material Name
                                </th>

                                <th>
                                    Quantity
                                </th>

                                <th>
                                    UoM
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of warehouse.inventory
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.materialName
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.uom
                                )}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `

                </div>

            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// INVENTORY CHECK
// ============================================================

async function inventoryCheck() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `
        <p>
            Checking inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory/check"
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `

                <div class="empty-message">

                    Inventory check failed.

                    <br><br>

                    ${escapeHtml(
                        data.message || ""
                    )}

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Inventory OK
        // ----------------------------------------------------

        if (data.valid) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        ✓ Inventory Check Passed
                    </h3>

                    <p>
                        No inconsistencies were found.
                    </p>

                    <p>
                        Warehouses checked:
                        <strong>
                            ${data.warehousesChecked}
                        </strong>
                    </p>

                    <p>
                        Inventory entries checked:
                        <strong>
                            ${data.entriesChecked}
                        </strong>
                    </p>

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Problems found
        // ----------------------------------------------------

        let html = `

            <div class="search-result">

                <h3>
                    Inventory Check
                </h3>

                <p>
                    The following inconsistencies were found:
                </p>

                <ul>
        `;


        for (
            const problem of data.problems
        ) {

            html += `

                <li>
                    ${escapeHtml(problem)}
                </li>

            `;
        }


        html += `

                </ul>

            </div>

        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// CREATE PRODUCT FORM
// ============================================================

function showCreateProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Product Name
            </label>

            <input
                type="text"
                id="product-name"
                placeholder="Enter Product Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="product-description"
                rows="4"
                placeholder="Enter Product Description"></textarea>


            <h3>
                Bill of Materials
            </h3>


            <div id="bom-items">
            </div>


            <button
                type="button"
                onclick="addBOMItemRow()">

                + Add Component

            </button>


            <div class="form-actions">

                <button
                    onclick="createProduct()">

                    Create Product

                </button>

            </div>


            <div id="product-message">
            </div>

        </div>
    `;
}

// ============================================================
// ADD BOM ITEM
// ============================================================

function addBOMItemRow() {

    const container =
        document.getElementById(
            "bom-items"
        );


    const row =
        document.createElement(
            "div"
        );

    row.className =
        "bom-row";


    row.innerHTML = `

        <input
            type="text"
            class="bom-material-id"
            placeholder="Material ID"
            maxlength="10"
        >


        <input
            type="number"
            class="bom-quantity"
            min="1"
            placeholder="Quantity"
        >


        <button
            type="button"
            onclick="this.parentElement.remove()">

            Remove

        </button>

    `;


    container.appendChild(row);
}

// ============================================================
// CREATE PRODUCT
// ============================================================

async function createProduct() {

    const id =
        document.getElementById(
            "product-id"
        ).value.trim();

    const name =
        document.getElementById(
            "product-name"
        ).value.trim();

    const description =
        document.getElementById(
            "product-description"
        ).value.trim();


    const message =
        document.getElementById(
            "product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validate Product ID
    // --------------------------------------------------------

    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    // --------------------------------------------------------
    // Validate required fields
    // --------------------------------------------------------

    if (!name) {

        message.textContent =
            "Product Name is required.";

        return;
    }


    // --------------------------------------------------------
    // Read BOM rows
    // --------------------------------------------------------

    const rows =
        document.querySelectorAll(
            ".bom-row"
        );


    const bom = [];


    for (const row of rows) {

        const materialID =
            row.querySelector(
                ".bom-material-id"
            ).value.trim();


        const quantity =
            Number(
                row.querySelector(
                    ".bom-quantity"
                ).value
            );


        if (!/^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/.test(materialID))
        {
            message.textContent =
                "Invalid Material ID in BOM.";

            return;
        }


        if (quantity <= 0)
        {
            message.textContent =
                "BOM quantity must be greater than zero.";

            return;
        }


        bom.push({

            materialID:
                materialID,

            quantity:
                quantity

        });
    }


    // --------------------------------------------------------
    // Create product object
    // --------------------------------------------------------

    const product = {

        id: id,

        name: name,

        description: description,

        bom: bom

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(product)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product created successfully.";

            clearInputFields([
                "product-id",
                "product-name",
                "product-description"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY PRODUCTS
// ============================================================

async function showDisplayProducts() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `
        <p>
            Loading products...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/products"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${escapeHtml(message)}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.products ||
            data.products.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No products available.
                </div>
            `;

            return;
        }


        let html = "";


        for (
            const product of data.products
        ) {

            html += `

                <div class="material-table-container">

                    <h2>
                        ${escapeHtml(product.id)}
                        -
                        ${escapeHtml(product.name)}
                    </h2>

                    <p>
                        ${escapeHtml(
                            product.description || ""
                        )}
                    </p>
            `;


            if (
                !product.bom ||
                product.bom.length === 0
            ) {

                html += `

                    <div class="empty-message">
                        No BOM components.
                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Name
                                </th>

                                <th>
                                    Quantity
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of product.bom
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.materialName || ""
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `
                </div>
            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE PRODUCT FORM
// ============================================================

function showDeleteProduct() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="delete-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <div class="form-actions">

                <button
                    onclick="deleteProduct()">

                    Delete Product

                </button>

            </div>


            <div id="delete-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE PRODUCT
// ============================================================

async function deleteProduct() {

    const id =
        document.getElementById(
            "delete-product-id"
        ).value.trim();


    const message =
        document.getElementById(
            "delete-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/products/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product deleted successfully.";

            document.getElementById(
                "delete-product-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// SELL PRODUCT FORM
// ============================================================

function showSellProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Sell Product
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="sell-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="sell-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity Sold
            </label>

            <input
                type="number"
                id="sell-quantity"
                min="1"
                placeholder="Enter Quantity"
            >


            <div class="form-actions">

                <button
                    onclick="sellProduct()">

                    Sell Product

                </button>

            </div>


            <div id="sell-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// SELL PRODUCT
// ============================================================

async function sellProduct() {

    const warehouseID =
        document.getElementById(
            "sell-warehouse-id"
        ).value;

    const productID =
        document.getElementById(
            "sell-product-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "sell-quantity"
        ).value;


    const message =
        document.getElementById(
            "sell-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!productIDPattern.test(productID)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const sale = {

        warehouseID:
            Number(warehouseID),

        productID:
            productID,

        quantity:
            Number(quantity)

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/sell",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(sale)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product sold successfully.";

            document.getElementById(
                "sell-quantity"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// READ FILE AS BASE64
// ============================================================

function readFileAsBase64(file) {

    return new Promise(
        (resolve, reject) => {

            const reader =
                new FileReader();

            reader.onload = () => {

                resolve(
                    reader.result
                );
            };

            reader.onerror = () => {

                reject(
                    new Error(
                        "Could not read image file."
                    )
                );
            };

            reader.readAsDataURL(file);
        }
    );
}

// ============================================================
// CLEAR INPUT FIELDS
// ============================================================

function clearInputFields(ids) {

    ids.forEach(id => {

        const element =
            document.getElementById(id);

        if (element) {

            element.value = "";

        }
    });
}

// ============================================================
// SET DATA LOGGING
// ============================================================

async function setDataLogging() {

    const checkbox =
        document.getElementById(
            "log-data-operations"
        );

    const message =
        document.getElementById(
            "data-logging-message"
        );


    try {

        const response =
            await fetch(
                "/api/data/logging",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            enabled:
                                checkbox.checked
                        })
                }
            );


        const data =
            await response.json();


        if (!response.ok) {

            message.textContent =
                data.message ||
                "Could not update logging setting.";

            return;
        }


        message.textContent =
            data.message;

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// BACK TO DASHBOARD
// ============================================================

function closeModule() {

    const dashboard =
        document.querySelector(
            ".dashboard"
        );

    const moduleView =
        document.getElementById(
            "module-view"
        );


    moduleView.classList.add(
        "hidden"
    );

    dashboard.style.display =
        "grid";
}

// ============================================================
// PRODUCT SEARCH COMBOBOX
// ============================================================
// Same pattern as the Material combobox, used by Production
// Projection to pick a Product by ID or name.

let productCache = null;

async function loadProductCache(forceRefresh) {

    if (productCache && !forceRefresh) {
        return productCache;
    }

    try {

        const response =
            await fetch("/api/products");

        const data =
            await response.json();

        productCache =
            data.products || [];
    }
    catch (error) {

        console.error(error);

        productCache = [];
    }

    return productCache;
}

async function initProductCombobox(prefix, selectedID) {

    await loadProductCache(true);

    const hiddenInput =
        document.getElementById(prefix + "-product-id");

    const searchInput =
        document.getElementById(prefix + "-product-id-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    hiddenInput.value = selectedID || "";
    searchInput.value = selectedID || "";
}

function renderProductOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-product-id-search");

    const optionsBox =
        document.getElementById(prefix + "-product-id-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const filterText =
        searchInput.value.trim().toLowerCase();

    const products =
        productCache || [];

    const matches =
        products.filter(product =>
            product.id.toLowerCase().includes(filterText) ||
            (product.name || "").toLowerCase().includes(filterText)
        );

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No products found
            </div>
        `;
    }
    else {

        optionsBox.innerHTML =
            matches.map(product => `
                <div class="combobox-option"
                    onmousedown="selectProductOption('${prefix}', '${escapeHtml(product.id)}')">
                    <strong>${escapeHtml(product.id)}</strong>
                    &nbsp;&mdash;&nbsp;${escapeHtml(product.name)}
                </div>
            `).join("");
    }

    optionsBox.classList.remove("hidden");
}

function handleProductSearchInput(prefix) {

    renderProductOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-product-id-search");

    const hiddenInput =
        document.getElementById(prefix + "-product-id");

    const products =
        productCache || [];

    const typedText =
        searchInput.value.trim();

    const exactMatch =
        products.find(product =>
            product.id === typedText
        );

    hiddenInput.value =
        exactMatch ? exactMatch.id : "";
}

function selectProductOption(prefix, id) {

    document.getElementById(
        prefix + "-product-id"
    ).value = id;

    document.getElementById(
        prefix + "-product-id-search"
    ).value = id;

    document.getElementById(
        prefix + "-product-id-options"
    ).classList.add("hidden");
}

function hideProductOptionsDelayed(prefix) {

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-product-id-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

function productComboboxHtml(prefix, placeholder) {

    return `
        <div class="combobox">

            <input
                type="text"
                id="${prefix}-product-id-search"
                autocomplete="off"
                placeholder="${placeholder || "Search by ID or name..."}"
                oninput="handleProductSearchInput('${prefix}')"
                onfocus="renderProductOptions('${prefix}')"
                onblur="hideProductOptionsDelayed('${prefix}')"
            >

            <input type="hidden" id="${prefix}-product-id">

            <div id="${prefix}-product-id-options"
                class="combobox-options hidden">
            </div>

        </div>
    `;
}

// ============================================================
// NEW PROJECTION - FORM
// ============================================================

function showNewProjection() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                New Production Projection
            </h2>

            <label>
                Product
            </label>

            ${productComboboxHtml("projection")}

            <label>
                Warehouse ID (destination for orders placed from this batch)
            </label>

            <input
                type="number"
                id="projection-warehouse-id"
                placeholder="Enter Warehouse ID"
            >

            <label>
                Manufacturing Deadline
            </label>

            <input
                type="date"
                id="projection-deadline"
            >

            <label>
                Quantity to Manufacture
            </label>

            <input
                type="number"
                id="projection-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <div class="form-actions">

                <button
                    onclick="createProjectionSubmit()">

                    Calculate &amp; Save Projection

                </button>

            </div>

            <div id="projection-message">
            </div>

        </div>
    `;

    initProductCombobox("projection");
}

// ============================================================
// NEW PROJECTION - SUBMIT
// ============================================================
// Creates a persisted Projection batch on the backend: BOM
// quantity x units to manufacture, minus current stock (summed
// across every warehouse), frozen at creation time. Only materials
// with an actual shortfall are stored.

async function createProjectionSubmit() {

    const productID =
        document.getElementById(
            "projection-product-id"
        ).value.trim();

    const warehouseID =
        document.getElementById(
            "projection-warehouse-id"
        ).value;

    const deadline =
        document.getElementById(
            "projection-deadline"
        ).value;

    const quantity =
        document.getElementById(
            "projection-quantity"
        ).value;

    const message =
        document.getElementById(
            "projection-message"
        );


    if (!productID) {

        message.textContent =
            "Please select a valid Product from the list.";

        return;
    }

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }

    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity to Manufacture must be greater than zero.";

        return;
    }


    message.textContent =
        "Calculating...";


    const payload = {

        productID: productID,

        warehouseID: Number(warehouseID),

        deadline: deadline,

        manufactureQuantity: Number(quantity)
    };


    try {

        const response =
            await fetch(
                "/api/projections/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(payload)
                }
            );

        const responseData =
            await response.json()
                .catch(() => null);

        if (response.ok &&
            responseData &&
            responseData.id) {

            message.textContent = "";

            await openProjectionDetail(
                responseData.id);
        }
        else {

            const responseText =
                (responseData && responseData.message) ||
                await response.text()
                    .catch(() => "");

            message.textContent =
                responseText ||
                "Enough stock for all materials - nothing to order.";
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROJECTIONS - LIST
// ============================================================

async function showProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Projections...</p>
    `;

    await displayProjections();
}

async function displayProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/projections");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const projections =
            (data.projections || []).filter(
                projection => !projection.completed
            );

        if (projections.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No active Projections. Use "New Projection" to create one, or check Archived Projections.
                </div>
            `;

            return;
        }

        let html =
            `<div class="procurement-list">`;

        for (const projection of projections) {
            html += renderProjectionCard(projection);
        }

        html += `</div>`;

        content.innerHTML = html;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PROJECTIONS - ARCHIVE (Completed / produced batches only)
// ============================================================

async function showArchivedProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Archived Projections...</p>
    `;

    await displayArchivedProjections();
}

async function displayArchivedProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/projections");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const projections =
            (data.projections || []).filter(
                projection => projection.completed === true
            );

        if (projections.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No Archived Projections yet.
                </div>
            `;

            return;
        }

        let html =
            `<div class="procurement-list">`;

        for (const projection of projections) {
            html += renderProjectionCard(projection);
        }

        html += `</div>`;

        content.innerHTML = html;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PROJECTIONS - STATUS BADGE CLASS
// ============================================================

function projectionStatusClass(status) {

    if (status === "Fully Ordered" ||
        status === "Completed") {
        return "status-completed";
    }

    return "status-partial";
}

// ============================================================
// PROJECTIONS - ORDER TIMING BADGE CLASS
// ============================================================

function timingStatusClass(status) {

    if (status === "Overdue") {
        return "status-inactive";
    }

    if (status === "On Time") {
        return "status-active";
    }

    return "status-ordered";
}

// ============================================================
// PROJECTIONS - CARD MARKUP
// ============================================================

function renderProjectionCard(projection) {

    const displayStatus =
        projection.completed ? "Completed" : projection.status;

    return `

        <div
            class="procurement-card projection-card"
            id="projection-${projection.id}"
            onclick="openProjectionDetail('${escapeHtml(projection.id)}')">

            <div class="procurement-card-header">

                <div>
                    <strong>${escapeHtml(projection.id)}</strong>
                    &mdash;
                    ${escapeHtml(projection.productID)}
                    ${escapeHtml(projection.productName || "")}
                </div>

                <span class="status-badge ${projectionStatusClass(displayStatus)}">
                    ${escapeHtml(displayStatus)}
                </span>

            </div>

            <div class="material-detail-grid procurement-grid">

                <div class="detail-field">
                    <div class="detail-label">Warehouse</div>
                    <div class="detail-value">${projection.warehouseID}</div>
                </div>

                <div class="detail-field">
                    <div class="detail-label">Deadline</div>
                    <div class="detail-value">${escapeHtml(projection.deadline || "")}</div>
                </div>

                <div class="detail-field">
                    <div class="detail-label">Qty to Manufacture</div>
                    <div class="detail-value">${projection.manufactureQuantity}</div>
                </div>

                <div class="detail-field">
                    <div class="detail-label">Created</div>
                    <div class="detail-value">${escapeHtml(projection.creationDate || "")}</div>
                </div>

            </div>

        </div>
    `;
}

// ============================================================
// PROJECTIONS - DETAIL (materials to order for this batch)
// ============================================================

async function openProjectionDetail(id) {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Projection...</p>
    `;

    try {

        const response =
            await fetch(`/api/projections/${id}`);

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const projection =
            await response.json();

        const readOnly = !!projection.completed;

        const displayStatus =
            readOnly ? "Completed" : projection.status;

        let html = `

            <button
                type="button"
                class="back-button"
                onclick="showProjections()">

                &larr; Back to Projections

            </button>

            <div class="material-table-container">

                <h2>
                    ${escapeHtml(projection.id)}
                    &mdash;
                    ${escapeHtml(projection.productID)}
                    ${escapeHtml(projection.productName || "")}
                </h2>

                <p>
                    Warehouse ${projection.warehouseID}
                    &mdash; Deadline: ${escapeHtml(projection.deadline || "-")}
                    &mdash; Qty to Manufacture: ${projection.manufactureQuantity}
                    &mdash; Status:
                    <span class="status-badge ${projectionStatusClass(displayStatus)}">
                        ${escapeHtml(displayStatus)}
                    </span>
                </p>

                ${readOnly ? `
                    <p>
                        Production confirmed: ${projection.producedQuantity} units on ${escapeHtml(projection.completionDate || "-")}.
                        BOM materials were issued from the Warehouse.
                    </p>
                ` : `
                    <div class="projection-delete-button-row">
                        <button
                            type="button"
                            class="button-danger"
                            onclick="deleteProjectionConfirm('${escapeHtml(projection.id)}')">

                            Delete Projection

                        </button>
                    </div>
                `}

                <div id="projection-detail-message">
                </div>

                <table class="material-table">

                    <thead>

                        <tr>
                            <th></th>
                            <th>Photo</th>
                            <th>Material ID</th>
                            <th>Name</th>
                            <th>UoM</th>
                            <th title="Warehouse stock minus what other active Projections have already reserved">Virtual Stock</th>
                            <th>Required</th>
                            <th>Pending</th>
                            <th>Order By</th>
                            <th>Status</th>
                            <th>Qty to Order</th>
                        </tr>

                    </thead>

                    <tbody>
        `;


        let anySelectable = false;

        for (const item of (projection.items || [])) {

            const rowID =
                "projdet-" + item.materialID;

            const alreadyOrdered =
                readOnly || item.pendingQuantity <= 0;

            // Once anything is already on order for this material
            // (from any Projection), replace the raw Pending number
            // with a status - a bare number next to an order that
            // already exists reads as "still needs everything",
            // which is confusing. The exact remaining amount is
            // still available, prefilled into "Qty to Order" below.

            const pendingCell =
                alreadyOrdered
                ? `<span class="status-badge status-active">&#10003; Fully ordered</span>`
                : (item.orderedQuantity > 0
                    ? `<span class="status-badge status-partial">Partially Ordered</span>`
                    : `<strong>${item.pendingQuantity}</strong>`);

            if (!alreadyOrdered) {
                anySelectable = true;
            }

            html += `

                <tr id="${rowID}-row">

                    <td>
                        ${
                            alreadyOrdered
                            ? ""
                            : `<input
                                    type="checkbox"
                                    id="${rowID}-select"
                               >`
                        }
                    </td>

                    <td>
                        ${
                            item.photo
                            ? `<img
                                    src="/${item.photo}"
                                    class="material-thumbnail"
                                    alt="Material photo"
                               >`
                            : "No photo"
                        }
                    </td>

                    <td>${escapeHtml(item.materialID)}</td>
                    <td>${escapeHtml(item.materialName || "")}</td>
                    <td>${escapeHtml(item.uom || "")}</td>
                    <td>${item.currentStock}</td>
                    <td>${item.requiredQuantity}</td>
                    <td>${pendingCell}</td>
                    <td>${escapeHtml(item.orderByDate || "-")}</td>

                    <td>
                        <span class="status-badge ${timingStatusClass(item.timingStatus)}">
                            ${escapeHtml(item.timingStatus || "")}
                        </span>
                    </td>

                    <td>
                        ${
                            alreadyOrdered
                            ? "-"
                            : `<input
                                    type="number"
                                    min="1"
                                    class="procurement-qty-input"
                                    id="${rowID}-qty"
                                    value="${item.pendingQuantity}"
                               >`
                        }
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

                ${anySelectable ? `

                    <div class="procurement-inline-form">

                        <label for="projdet-order-date">
                            Order Date
                        </label>

                        <input
                            type="date"
                            id="projdet-order-date"
                        >

                        <button
                            type="button"
                            onclick="registerSelectedOrders('${escapeHtml(projection.id)}', '${escapeHtml(projection.productID)}', ${projection.warehouseID})">

                            Register Order

                        </button>

                        <button
                            type="button"
                            onclick="registerSelectedOrdersToday('${escapeHtml(projection.id)}', '${escapeHtml(projection.productID)}', ${projection.warehouseID})">

                            Register Today

                        </button>

                    </div>

                ` : ""}

            </div>

            ${(!readOnly && projection.status === "Fully Ordered") ? `

                <div class="procurement-section procurement-section-standalone">

                    <h3>
                        Confirm Production
                    </h3>

                    <p>
                        Confirming production issues the full Bill of
                        Materials for the produced quantity from the
                        Warehouse (Goods Issue) and archives this
                        Projection into Archived Projections.
                    </p>

                    <div class="procurement-inline-form">

                        <label for="projdet-produced-qty">
                            Produced Quantity
                        </label>

                        <input
                            type="number"
                            min="1"
                            id="projdet-produced-qty"
                            value="${projection.manufactureQuantity}"
                        >

                        <label for="projdet-completion-date">
                            Completion Date
                        </label>

                        <input
                            type="date"
                            id="projdet-completion-date"
                        >

                        <button
                            type="button"
                            onclick="confirmProduction('${escapeHtml(projection.id)}')">

                            Confirm Production

                        </button>

                    </div>

                </div>

            ` : ""}
        `;

        content.innerHTML = html;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PROJECTIONS - CONFIRM PRODUCTION (issue BOM, archive batch)
// ============================================================

async function confirmProduction(id) {

    const message =
        document.getElementById(
            "projection-detail-message"
        );

    const producedQuantity =
        document.getElementById(
            "projdet-produced-qty"
        ).value;

    const completionDate =
        document.getElementById(
            "projdet-completion-date"
        ).value;

    if (!producedQuantity ||
        Number(producedQuantity) <= 0) {

        message.textContent =
            "Please enter a Produced Quantity greater than zero.";

        return;
    }

    if (!completionDate) {

        message.textContent =
            "Please enter a Completion Date.";

        return;
    }

    const confirmed = await showConfirmDialog(
        "This will issue the full Bill of Materials for " +
        producedQuantity +
        " unit(s) from the Warehouse and archive this Projection. Continue?"
    );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/projections/complete",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        id: id,
                        producedQuantity: Number(producedQuantity),
                        completionDate: completionDate
                    })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showProjections();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROJECTIONS - DELETE
// ============================================================
// Scraps the plan outright. Procurement Orders already placed from
// it are not touched (see the backend) - only the Projection itself
// disappears from the active list.

async function deleteProjectionConfirm(id) {

    const message =
        document.getElementById(
            "projection-detail-message"
        );

    const confirmed = await showConfirmDialog(
        `Delete Projection ${id}? This cannot be undone. ` +
        "Procurement Orders already placed from it will not be deleted."
    );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/projections/delete",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showProjections();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROJECTIONS - REGISTER ORDER FOR SELECTED MATERIALS (ONE PRC)
// ============================================================
// Every checked material is registered together under one shared
// "PRC-######" number and one Order Date - one action, one
// Procurement Order, instead of a separate PRC per material.

// ============================================================
// PROJECTION DETAIL - REGISTER ORDER, TODAY (quick convenience)
// ============================================================
// Same one-click convenience as "Confirm as Ordered Today"/"Receipt
// Today": fills the Order Date with today and reuses the normal
// registration path so validation/behavior stays identical.

function registerSelectedOrdersToday(projectionID, productID, warehouseID) {

    const dateInput =
        document.getElementById(
            "projdet-order-date"
        );

    if (!dateInput) {
        return;
    }

    const today =
        new Date().toISOString().slice(0, 10);

    dateInput.value = today;

    registerSelectedOrders(projectionID, productID, warehouseID);
}

async function registerSelectedOrders(projectionID, productID, warehouseID) {

    const message =
        document.getElementById(
            "projection-detail-message"
        );

    const orderDate =
        document.getElementById(
            "projdet-order-date"
        ).value;

    if (!orderDate) {

        message.textContent =
            "Please enter an Order Date.";

        return;
    }

    const checkboxes =
        document.querySelectorAll(
            '[id^="projdet-"][id$="-select"]'
        );

    const lines = [];

    for (const checkbox of checkboxes) {

        if (!checkbox.checked) {
            continue;
        }

        const materialID =
            checkbox.id.slice(
                "projdet-".length,
                checkbox.id.length - "-select".length
            );

        const qtyInput =
            document.getElementById(
                "projdet-" + materialID + "-qty"
            );

        const quantity =
            qtyInput ? qtyInput.value : "";

        if (!quantity ||
            Number(quantity) <= 0) {

            message.textContent =
                `Quantity to Order must be greater than zero for material ${materialID}.`;

            return;
        }

        lines.push({
            materialID: materialID,
            orderedQuantity: Number(quantity)
        });
    }

    if (lines.length === 0) {

        message.textContent =
            "Select at least one material to order.";

        return;
    }


    const payload = {

        productID: productID,

        warehouseID: Number(warehouseID),

        orderDate: orderDate,

        comment: "",

        projectionID: projectionID,

        lines: lines
    };


    try {

        const response =
            await fetch(
                "/api/procurement/create-batch",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(payload)
                }
            );

        const responseText =
            await response.text();

        if (response.ok) {

            message.textContent = "";

            // Refresh the whole batch so "Pending" for every line
            // reflects the new order right away.

            await openProjectionDetail(
                projectionID);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROCUREMENT ORDERS - LIST (Open Orders / Confirmed Orders)
// ============================================================
// The active (not yet Completed) orders are split into two views by
// their combined status: Open Orders are still waiting on supplier
// confirmation (status "Ordered"), Confirmed Orders are confirmed
// and now just waiting on goods receipt ("Confirmed" or "Partially
// Received"). Both share the same fetch/group/render pipeline below,
// distinguished only by "stage" - kept in currentProcurementStage so
// every refresh call (after confirming, receiving, etc.) redraws
// whichever of the two views is actually open, without every call
// site needing to pass it explicitly.

let currentProcurementStage = "open";

async function showProcurementOrders() {

    currentProcurementStage = "open";

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Open Orders...</p>
    `;

    await displayProcurementOrders();
}

async function showConfirmedOrders() {

    currentProcurementStage = "confirmed";

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Confirmed Orders...</p>
    `;

    await displayProcurementOrders();
}

async function displayProcurementOrders() {

    const stage =
        currentProcurementStage;

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/procurement");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        // Completed orders move to the Archived Orders view instead
        // of staying in either active list.

        const lines =
            (data.orders || []).filter(
                order => order.status !== "Completed" &&
                         order.status !== "Closed (Incomplete)" &&
                         order.status !== "Cancelled"
            );

        const groups =
            groupProcurementLines(lines)
                .filter(group => {

                    const status =
                        combineProcurementStatus(group.lines);

                    return stage === "open"
                        ? status === "Ordered"
                        : (status === "Confirmed" ||
                           status === "Partially Received");
                });

        if (groups.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    ${stage === "open"
                        ? `No Open Orders. Orders waiting on
                           supplier confirmation appear here.`
                        : `No Confirmed Orders. Orders confirmed by
                           the supplier and waiting on goods receipt
                           appear here.`}
                </div>
            `;

            return;
        }

        content.innerHTML =
            renderProcurementGroupedList(groups, stage);
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// NEW PROCUREMENT ORDER - FORM (from scratch, not tied to a
// Projection - one or more materials under one shared "PRC-######")
// ============================================================

function showNewProcurementOrder() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                New Procurement Order
            </h2>

            <label>
                Product (optional - for reference only)
            </label>

            ${productComboboxHtml("neworder", "Search by ID or name (optional)...")}

            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="neworder-warehouse-id"
                placeholder="Enter Warehouse ID"
            >

            <label>
                Order Date
            </label>

            <input
                type="date"
                id="neworder-order-date"
            >

            <label>
                Comment (optional)
            </label>

            <input
                type="text"
                id="neworder-comment"
                placeholder="Enter a comment"
            >

            <label>
                Materials
            </label>

            <div id="neworder-lines">

                ${neworderRowHtml(0)}

            </div>

            <button
                type="button"
                onclick="addProcurementOrderRow()">

                + Add Material

            </button>

            <div class="form-actions">

                <button
                    onclick="submitNewProcurementOrder()">

                    Create Order

                </button>

            </div>

            <div id="neworder-message">
            </div>

        </div>
    `;

    neworderLineCounter = 1;

    initProductCombobox("neworder");

    initMaterialCombobox("neworder-line-0");
}

// ============================================================
// NEW PROCUREMENT ORDER - MATERIAL ROW MARKUP (combobox-based)
// ============================================================

let neworderLineCounter = 0;

function neworderRowHtml(rowIndex) {

    const prefix =
        `neworder-line-${rowIndex}`;

    return `
        <div class="bom-row" id="${prefix}-row">

            ${materialComboboxHtml(prefix, "Search by ID or name...")}

            <input
                type="number"
                class="neworder-quantity"
                min="1"
                placeholder="Quantity"
            >

            <button
                type="button"
                onclick="document.getElementById('${prefix}-row').remove()">

                Remove

            </button>

        </div>
    `;
}

// ============================================================
// NEW PROCUREMENT ORDER - ADD MATERIAL ROW
// ============================================================

function addProcurementOrderRow() {

    const container =
        document.getElementById(
            "neworder-lines"
        );

    const rowIndex =
        neworderLineCounter++;

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        neworderRowHtml(rowIndex).trim();

    container.appendChild(
        wrapper.firstElementChild
    );

    initMaterialCombobox(
        `neworder-line-${rowIndex}`
    );
}

// ============================================================
// NEW PROCUREMENT ORDER - SUBMIT
// ============================================================
// Creates a standalone Procurement Order (or several material lines
// sharing one "PRC-######"), with no Projection attached - straight
// from Procurement Orders, for material that just needs restocking.

async function submitNewProcurementOrder() {

    const message =
        document.getElementById(
            "neworder-message"
        );

    const productID =
        document.getElementById(
            "neworder-product-id"
        ).value.trim();

    const warehouseID =
        document.getElementById(
            "neworder-warehouse-id"
        ).value;

    const orderDate =
        document.getElementById(
            "neworder-order-date"
        ).value;

    const comment =
        document.getElementById(
            "neworder-comment"
        ).value.trim();


    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }

    if (!orderDate) {

        message.textContent =
            "Please enter an Order Date.";

        return;
    }


    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const rows =
        document.querySelectorAll(
            "#neworder-lines .bom-row"
        );

    const lines = [];

    for (const row of rows) {

        const materialIDInput =
            row.querySelector(
                'input[type="hidden"][id$="-material-id"]'
            );

        const materialID =
            materialIDInput ?
            materialIDInput.value.trim() : "";

        const quantity =
            Number(
                row.querySelector(
                    ".neworder-quantity"
                ).value
            );

        if (!materialID && !quantity) {
            continue;
        }

        if (!materialIDPattern.test(materialID)) {

            message.textContent =
                `Invalid Material ID: ${materialID || "(empty)"}.`;

            return;
        }

        if (!quantity || quantity <= 0) {

            message.textContent =
                `Quantity must be greater than zero for material ${materialID}.`;

            return;
        }

        lines.push({
            materialID: materialID,
            orderedQuantity: quantity
        });
    }

    if (lines.length === 0) {

        message.textContent =
            "Add at least one material with a quantity.";

        return;
    }


    const payload = {

        productID: productID,

        warehouseID: Number(warehouseID),

        orderDate: orderDate,

        comment: comment,

        projectionID: "",

        lines: lines
    };


    try {

        const response =
            await fetch(
                "/api/procurement/create-batch",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(payload)
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showProcurementOrders();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROCUREMENT ORDERS - ARCHIVE (Completed orders only)
// ============================================================

async function showArchivedOrders() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Archived Orders...</p>
    `;

    await displayArchivedOrders();
}

async function displayArchivedOrders() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/procurement");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const lines =
            (data.orders || []).filter(
                order => order.status === "Completed" ||
                         order.status === "Closed (Incomplete)" ||
                         order.status === "Cancelled"
            );

        if (lines.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No Archived (Completed) Procurement Orders yet.
                </div>
            `;

            return;
        }

        content.innerHTML =
            renderProcurementGroupedList(
                groupProcurementLines(lines),
                "archived");
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PROCUREMENT ORDERS - GROUP LINES INTO ORDERS, THEN BY SUPPLIER
// ============================================================
// One "PRC-######" id can now cover several materials (lines) -
// one action, one Procurement Order. Lines sharing an id are
// grouped into a single card; groups are then bucketed by Supplier
// (taken from the group's first line) the same way single-material
// orders were before.

function groupProcurementLines(lines) {

    const groups = [];
    const indexByID = {};

    for (const line of lines) {

        if (!(line.id in indexByID)) {

            indexByID[line.id] = groups.length;

            groups.push({
                id: line.id,
                lines: []
            });
        }

        groups[indexByID[line.id]].lines.push(line);
    }

    return groups;
}

function renderProcurementGroupedList(groups, cardMode) {

    const groupsBySupplier = {};

    for (const group of groups) {

        const supplierName =
            group.lines[0].supplierName || "No Supplier";

        if (!groupsBySupplier[supplierName]) {
            groupsBySupplier[supplierName] = [];
        }

        groupsBySupplier[supplierName].push(group);
    }

    const supplierNames =
        Object.keys(groupsBySupplier).sort((a, b) => {

            if (a === "No Supplier") return 1;
            if (b === "No Supplier") return -1;

            return a.localeCompare(b);
        });


    let html = "";

    for (const supplierName of supplierNames) {

        html += `
            <h2 class="supplier-group-heading">
                ${escapeHtml(supplierName)}
            </h2>

            <div class="procurement-list">
        `;

        for (const group of groupsBySupplier[supplierName]) {
            html += renderProcurementCard(group, cardMode);
        }

        html += `</div>`;
    }

    return html;
}

// ============================================================
// PROCUREMENT ORDERS - STATUS BADGE CLASS / COMBINING
// ============================================================

function procurementStatusClass(status) {

    if (status === "Cancelled") {
        return "status-cancelled";
    }

    if (status === "Closed (Incomplete)") {
        return "status-closed";
    }

    if (status === "Completed") {
        return "status-completed";
    }

    if (status === "Partially Received") {
        return "status-partial";
    }

    if (status === "Confirmed") {
        return "status-confirmed";
    }

    return "status-ordered";
}

// One combined status for a whole order (several material lines):
// "Closed (Incomplete)" wins if any line was closed that way (the
// order as a whole is not cleanly done); otherwise "Partially
// Received" wins whenever any line is (something has already
// arrived, but the order isn't done); otherwise the least advanced
// line's status represents the whole order, since that is what still
// needs attention.

function combineProcurementStatus(lines) {

    const stages =
        ["Ordered", "Confirmed", "Partially Received", "Completed"];

    if (lines.some(line => line.status === "Cancelled")) {
        return "Cancelled";
    }

    if (lines.some(line => line.status === "Closed (Incomplete)")) {
        return "Closed (Incomplete)";
    }

    if (lines.some(line => line.status === "Partially Received")) {
        return "Partially Received";
    }

    let earliestIndex = stages.length - 1;

    for (const line of lines) {

        const index =
            stages.indexOf(line.status);

        if (index !== -1 && index < earliestIndex) {
            earliestIndex = index;
        }
    }

    return stages[earliestIndex];
}

// ============================================================
// PROCUREMENT ORDERS - CARD MARKUP (one order, 1+ material lines)
// ============================================================

function renderProcurementCard(group, cardMode = "open") {

    const id = group.id;
    const lines = group.lines;

    const combinedStatus =
        combineProcurementStatus(lines);

    const totalOrdered =
        lines.reduce((sum, line) => sum + line.orderedQuantity, 0);

    const totalReceived =
        lines.reduce((sum, line) => sum + line.totalReceivedQuantity, 0);

    const materialsLabel =
        lines.length === 1
        ? `${lines[0].materialID} ${lines[0].materialName || ""}`
        : lines.map(line => line.materialID).join(", ");

    const warehouseID =
        lines[0].warehouseID;

    // One shared Confirmation Date for the whole order, but each
    // material line keeps its own Confirmed Quantity - use whichever
    // line already has a date saved (they're kept in sync by
    // saveGroupConfirmation).

    const sharedConfirmationDate =
        (lines.find(line => line.confirmationDate) || {})
            .confirmationDate || "";


    // One block per material: Order Date / Ordered Qty /
    // Received-Pending, plus (mode-dependent) either its own Qty
    // field right there in the same row (Open Orders - so the field
    // to confirm sits next to that material's own info instead of a
    // separate section further down), or its own Add Receipt fields
    // directly underneath (Confirmed Orders), plus its receipt
    // history if it has any. Only labeled with the material when an
    // order covers more than one.

    let materialRowsHtml = "";

    for (const line of lines) {

        const materialHeaderHtml =
            lines.length > 1
            ? `
                <div class="procurement-line-header">
                    <strong>${escapeHtml(line.materialID)}</strong>
                    ${escapeHtml(line.materialName || "")}
                </div>
            `
            : "";

        const confirmQtyFieldHtml =
            cardMode === "open"
            ? `
                <div class="detail-field">
                    <div class="detail-label">Qty</div>
                    <input
                        type="number"
                        min="0"
                        class="procurement-qty-input"
                        id="${id}-${line.materialID}-confirm-qty"
                        placeholder="Qty (0 = delete)"
                        value="${line.confirmedQuantity > 0 ? line.confirmedQuantity : ""}"
                        data-ordered-qty="${line.orderedQuantity}"
                    >
                </div>
            `
            : "";

        let addReceiptHtml = "";

        if (cardMode === "confirmed" && line.pendingQuantity > 0) {

            addReceiptHtml = `

                <div class="procurement-section">

                    <div class="detail-label">
                        Add Receipt
                    </div>

                    <div class="procurement-inline-form">

                        <input
                            type="date"
                            id="${id}-${line.materialID}-receipt-date"
                        >

                        <input
                            type="number"
                            min="1"
                            max="${line.pendingQuantity}"
                            class="procurement-qty-input"
                            id="${id}-${line.materialID}-receipt-qty"
                            placeholder="Receive"
                        >

                        <input
                            type="text"
                            id="${id}-${line.materialID}-receipt-comment"
                            placeholder="Comment (optional)"
                        >

                    </div>

                </div>
            `;
        }

        let receiptsHtml = "";

        if (cardMode !== "open" &&
            line.receipts && line.receipts.length > 0) {

            receiptsHtml = `
                <div class="procurement-receipts">

                    <div class="detail-label">
                        Receipt History
                    </div>

                    ${line.receipts.map(receipt => `
                        <div class="procurement-receipt-row">
                            ${escapeHtml(receipt.receiptDate)}
                            &mdash;
                            ${receipt.receivedQuantity}
                            ${receipt.comment
                                ? "(" + escapeHtml(receipt.comment) + ")"
                                : ""}
                        </div>
                    `).join("")}

                </div>
            `;
        }

        materialRowsHtml += `

            <div class="procurement-line">

                ${materialHeaderHtml}

                <div class="material-detail-grid procurement-grid procurement-grid-compact">

                    <div class="detail-field">
                        <div class="detail-label">Order Date</div>
                        <div class="detail-value">${escapeHtml(line.orderDate)}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Ordered Qty</div>
                        <div class="detail-value">${line.orderedQuantity}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Received / Pending</div>
                        <div class="detail-value">${line.totalReceivedQuantity} / ${line.pendingQuantity}</div>
                    </div>

                    ${confirmQtyFieldHtml}

                </div>

                ${addReceiptHtml}

                ${receiptsHtml}

            </div>
        `;
    }


    // Confirmation (Open Orders only): one shared Order Date and pair
    // of actions for the whole order plus the resulting status badge -
    // the per-material Qty fields now live in each material's own row
    // above, not here.

    let confirmationHtml = "";

    if (cardMode === "open") {

        confirmationHtml = `

            <div class="procurement-section">

                <div class="detail-label">
                    Confirmation
                </div>

                <div class="procurement-inline-form">

                    <input
                        type="date"
                        id="${id}-confirm-date"
                        value="${escapeHtml(sharedConfirmationDate)}"
                    >

                    <button
                        type="button"
                        onclick="saveGroupConfirmation('${id}')">

                        Confirm

                    </button>

                    <button
                        type="button"
                        onclick="quickConfirmGroup('${id}')">

                        Confirm as Ordered Today

                    </button>

                    <button
                        type="button"
                        class="button-danger"
                        onclick="cancelOrderGroup('${id}')">

                        Cancel

                    </button>

                    <span class="status-badge procurement-confirm-status ${procurementStatusClass(combinedStatus)}">
                        ${escapeHtml(combinedStatus)}
                    </span>

                </div>

            </div>
        `;
    }


    // Material Receipt (Confirmed Orders only): one shared button
    // under every material's own Add Receipt fields - one click
    // receives whichever materials were given a quantity above.

    let receiptButtonHtml = "";

    if (cardMode === "confirmed" &&
        lines.some(line => line.pendingQuantity > 0)) {

        receiptButtonHtml = `

            <div class="procurement-inline-form">

                <button
                    type="button"
                    class="button-danger"
                    onclick="closeOrderGroup('${id}')">

                    Close Order

                </button>

                <button
                    type="button"
                    onclick="saveGroupReceipts('${id}')">

                    Material Receipt

                </button>

                <button
                    type="button"
                    onclick="quickReceiptGroup('${id}')">

                    Receipt Today

                </button>

            </div>
        `;
    }

    return `

        <div class="procurement-card collapsed" id="order-${id}">

            <div
                class="procurement-summary"
                onclick="toggleProcurementCard('${id}')">

                <span class="procurement-summary-id">
                    ${escapeHtml(id)}
                </span>

                <span class="procurement-summary-material">
                    ${escapeHtml(materialsLabel)}
                </span>

                <span class="procurement-summary-qty">
                    WH ${warehouseID}
                    &nbsp;&middot;&nbsp;
                    ${totalReceived} / ${totalOrdered}
                </span>

                <span class="status-badge ${procurementStatusClass(combinedStatus)}">
                    ${escapeHtml(combinedStatus)}
                </span>

                <span class="procurement-summary-caret">
                    &#9656;
                </span>

            </div>

            <div class="procurement-body" id="order-${id}-body" hidden>

                ${materialRowsHtml}

                ${confirmationHtml}

                ${receiptButtonHtml}

                <div id="${id}-message" class="procurement-card-message">
                </div>

            </div>

        </div>
    `;
}

// ============================================================
// PROCUREMENT ORDERS - TOGGLE (expand/collapse in place)
// ============================================================

function toggleProcurementCard(id) {

    const card =
        document.getElementById("order-" + id);

    const body =
        document.getElementById("order-" + id + "-body");

    if (!card || !body) {
        return;
    }

    body.hidden = !body.hidden;

    card.classList.toggle(
        "collapsed",
        body.hidden);
}

// ============================================================
// PROCUREMENT ORDERS - QUICK CONFIRM (today, same ordered qty)
// ============================================================
// One click for the common case: supplier confirmed exactly what
// was ordered, today, for every material in this order. Fills the
// Confirmation fields and reuses the normal save path so
// validation/behavior stays identical.

function quickConfirmGroup(id) {

    const dateInput =
        document.getElementById(
            id + "-confirm-date"
        );

    if (!dateInput) {
        return;
    }

    const today =
        new Date().toISOString().split("T")[0];

    dateInput.value = today;

    const qtyInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="-confirm-qty"]`
        );

    for (const input of qtyInputs) {
        input.value = input.dataset.orderedQty;
    }

    saveGroupConfirmation(id);
}

// ============================================================
// PROCUREMENT ORDERS - SAVE CONFIRMATION (whole order at once)
// ============================================================
// One Confirmation Date applies to every material line in this
// order, but each line keeps its own Confirmed Quantity. Saved one
// line at a time against the existing /api/procurement/confirm
// endpoint (which already deletes a line outright when its
// confirmed quantity is 0) so server-side behavior/validation stays
// exactly as it is for a single-material order.

async function saveGroupConfirmation(id) {

    const message =
        document.getElementById(
            id + "-message"
        );

    const confirmationDate =
        document.getElementById(
            id + "-confirm-date"
        ).value;

    if (!confirmationDate) {

        message.textContent =
            "Please enter a Confirmation Date.";

        return;
    }

    const qtyInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="-confirm-qty"]`
        );

    const lineUpdates = [];

    for (const input of qtyInputs) {

        const materialID =
            input.id.slice(
                id.length + 1,
                input.id.length - "-confirm-qty".length
            );

        if (input.value === "" ||
            Number(input.value) < 0) {

            message.textContent =
                `Please enter a Confirmed Quantity for ${materialID} ` +
                `(0 deletes that material from the order).`;

            return;
        }

        lineUpdates.push({
            materialID: materialID,
            confirmedQuantity: Number(input.value)
        });
    }

    const toDelete =
        lineUpdates.filter(line => line.confirmedQuantity === 0);

    if (toDelete.length > 0) {

        const confirmed =
            await showConfirmDialog(
                "Setting the Confirmed Quantity to 0 will remove " +
                toDelete.map(line => line.materialID).join(", ") +
                ` from Procurement Order ${id}. This cannot be undone. ` +
                "Continue?"
            );

        if (!confirmed) {
            return;
        }
    }

    message.textContent = "";

    for (const line of lineUpdates) {

        try {

            const response =
                await fetch(
                    "/api/procurement/confirm",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify({
                            id: id,
                            materialID: line.materialID,
                            confirmationDate: confirmationDate,
                            confirmedQuantity: line.confirmedQuantity
                        })
                    }
                );

            if (!response.ok) {

                const responseText =
                    await response.text();

                message.textContent =
                    `Error (${line.materialID}): ` + responseText;

                await displayProcurementOrders();

                return;
            }
        }
        catch (error) {

            console.error(error);

            message.textContent =
                "Could not connect to the server.";

            return;
        }
    }

    await displayProcurementOrders();
}

// ============================================================
// PROCUREMENT ORDERS - QUICK RECEIPT (today, whatever Qty is filled)
// ============================================================
// Same one-click convenience as "Confirm as Ordered (Today)": fills
// today's date into every material's Receipt Date field, then reuses
// the normal save path - the operator still enters each Qty by hand
// (there is no "ordered quantity" default here, since a receipt is
// often partial).

function quickReceiptGroup(id) {

    const today =
        new Date().toISOString().split("T")[0];

    const dateInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="-receipt-date"]`
        );

    for (const input of dateInputs) {
        input.value = today;
    }

    saveGroupReceipts(id);
}

// ============================================================
// PROCUREMENT ORDERS - CLOSE ORDER (won't fully arrive)
// ============================================================
// For when the rest of an order is simply never coming (goods lost,
// discontinued, cancelled after a partial delivery...). Closes every
// still-pending material line under this order in one action -
// whatever was already received stays in inventory - and the order
// moves out of Confirmed Orders into Archived Orders, marked
// "Closed (Incomplete)".

async function closeOrderGroup(id) {

    const message =
        document.getElementById(
            `${id}-message`
        );

    const confirmed =
        await showConfirmDialog(
            `Close Procurement Order ${id}? Whatever has already ` +
            "been received stays in inventory, but the rest will no " +
            "longer count as pending and this order will move to " +
            "Archived Orders as \"Closed (Incomplete)\". This cannot " +
            "be undone."
        );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/procurement/close",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorText =
                await response.text();

            if (message) {
                message.textContent =
                    "Error: " + errorText;
            }

            return;
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}

// ============================================================
// PROCUREMENT ORDERS - CANCEL ORDER (never confirmed)
// ============================================================
// For an order still sitting in Open Orders that will never be
// confirmed by the supplier. Moves it to Archived Orders as
// "Cancelled" - distinct from "Closed (Incomplete)", which is for
// an order that was confirmed and partially received.

async function cancelOrderGroup(id) {

    const message =
        document.getElementById(
            `${id}-message`
        );

    const confirmed =
        await showConfirmDialog(
            `Cancel Procurement Order ${id}? This order will move to ` +
            "Archived Orders as \"Cancelled\". This cannot be undone."
        );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/procurement/cancel",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorText =
                await response.text();

            if (message) {
                message.textContent =
                    "Error: " + errorText;
            }

            return;
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}

// ============================================================
// PROCUREMENT ORDERS - SAVE RECEIPT (triggers Goods Receipt)
// ============================================================
// One click for the whole order: every material line in the Add
// Receipt block that was given a quantity is received, in turn,
// through the same /api/procurement/receive endpoint used before -
// only now the operator does not need to click a separate button
// per material.

async function saveGroupReceipts(id) {

    const message =
        document.getElementById(
            `${id}-message`
        );

    if (message) {
        message.textContent = "";
    }

    const qtySuffix =
        "-receipt-qty";

    const qtyInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="${qtySuffix}"]`
        );

    const toSubmit = [];

    for (const qtyInput of qtyInputs) {

        const materialID =
            qtyInput.id.slice(
                id.length + 1,
                qtyInput.id.length - qtySuffix.length
            );

        const rawQty =
            qtyInput.value;

        if (!rawQty ||
            Number(rawQty) <= 0) {
            continue;
        }

        const dateInput =
            document.getElementById(
                `${id}-${materialID}-receipt-date`
            );

        const commentInput =
            document.getElementById(
                `${id}-${materialID}-receipt-comment`
            );

        if (!dateInput || !dateInput.value) {

            if (message) {
                message.textContent =
                    `Please enter a Receipt Date for ${materialID}.`;
            }

            return;
        }

        toSubmit.push({
            materialID: materialID,
            receiptDate: dateInput.value,
            receivedQuantity: Number(rawQty),
            comment: commentInput ?
                commentInput.value.trim() : ""
        });
    }

    if (toSubmit.length === 0) {

        if (message) {
            message.textContent =
                "Enter a received quantity for at least one material.";
        }

        return;
    }

    try {

        for (const entry of toSubmit) {

            const response =
                await fetch(
                    "/api/procurement/receive",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify({
                            id: id,
                            materialID: entry.materialID,
                            receiptDate: entry.receiptDate,
                            receivedQuantity: entry.receivedQuantity,
                            comment: entry.comment
                        })
                    }
                );

            if (!response.ok) {

                const errorText =
                    await response.text();

                await displayProcurementOrders();

                const refreshedMessage =
                    document.getElementById(
                        `${id}-message`
                    );

                if (refreshedMessage) {
                    refreshedMessage.textContent =
                        `Error (${entry.materialID}): ${errorText}`;
                }

                return;
            }
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}